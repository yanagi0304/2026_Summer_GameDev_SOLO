#include "Enemy.h"

#include <cmath>

#include "../../pch.h"

#include "../../Utility/Utility.h"

#include "../../Manager/TimeScale/TimeScale.h"

#include "../Common/Collider/CapsuleCollider.h"
#include "../Common/PhysicsBody/PhysicsBody.h"
#include "../Common/MotionController/GroundMotionController.h"
#include "../Common/AnimationController/AnimationController.h"

#include "Navigation/NavGrid.h"
#include "State/EnemyIdleState.h"
#include "State/EnemyChaseState.h"

namespace
{
	// 単位はcm(プレイヤーのコライダー半径40、全高220に合わせる)

	// 視界の距離
	constexpr float SIGHT_RANGE = 2500.0f;

	// 視野角(正面から左右それぞれ)。追跡中は全方位を見られるものとする
	constexpr float SIGHT_HALF_ANGLE_DEG = 60.0f;

	// 追跡中は、見失いにくいように視界の距離を伸ばす倍率
	constexpr float CHASE_SIGHT_RANGE_RATE = 1.5f;

	// 見失ってから、最後に見た位置を探し続けるフレーム数
	constexpr float LOSE_SIGHT_FRAMES = 180.0f;

	// 目の高さ / 目標を狙う高さ(足元からの高さ)
	constexpr float EYE_HEIGHT = 160.0f;
	constexpr float TARGET_AIM_HEIGHT = 120.0f;
}

Enemy::Enemy(const NavContext& nav, const Vector3* targetPos, const Vector3& spawnPos) :
	CharacterBase(),

	nav(nav),
	targetPos(targetPos),
	spawnPos(spawnPos),

	chaseState(nullptr),

	canSeeTarget(false),
	hasLastKnownPos(false),
	lastKnownPos(),
	lostFrames(0.0f)
{
}

void Enemy::Load(void)
{
#pragma region オブジェクト設定

	// 動的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Movable);

	// 物理挙動の有効化
	EnablePhysics();
	GetPhysicsBody()->SetGravityEnabled(true);
	GetPhysicsBody()->SetRestitution(0.0f);
	GetPhysicsBody()->SetMode(PhysicsBody::MotionMode::Character);

	// 当たり判定による押し出しを有効にする
	SetContactResponseEnabled(true);
	GetPhysicsBody()->SetMass(1.0f);

	// 移動操作(プレイヤーと同じ設定。最高速度は追跡状態が設定する)
	GroundMotionController* motion = new GroundMotionController();
	AddMotionController(motion);
	motion->SetTransform(&trans);
	motion->SetGroundedSource(&isGround);
	motion->SetGroundAcceleration(3.0f);
	motion->SetAirAcceleration(3.0f);
	motion->SetMomentumSteering(0.2f);
	motion->SetMomentumThreshold(40.0f);

#pragma endregion

#pragma region モデル設定

	// 仮のモデル(プレイヤーと同じ)
	trans.LoadModel("Character/WoChamolenium/WoChamolenium");
	trans.SetLocalRotation(Quaternion::FromRotationY(Deg2Rad(180.0f)));

#pragma endregion

#pragma region アニメーション設定

	CreateAnimationController();
	AddInFbxAnimation(CHARACTER_ANIME::MAX, ANIME_SPEED_TABLE, ANIME_LOOP_TABLE);
	anime->SetPositionCorrectionFrame("mixamorig:Hips");
	anime->SetPositionCorrection(true);

#pragma endregion

#pragma region コライダー生成

	AddCollider(
		new CapsuleCollider(
			COLLIDER_TAG::Enemy,
			Vector3(0.0f, 35.0f, 0.0f),
			Vector3(0.0f, 220.0f, 0.0f),
			40.0f
		)
	);

#pragma endregion

#pragma region 状態設定

	// 待機状態
	AddState(
		STATE::Idle,
		new EnemyIdleState(
			[this]() { return ShouldChase(); },
			[this]() { AnimePlay(CHARACTER_ANIME::IDLE); }
		)
	);

	// 追跡状態
	chaseState = new EnemyChaseState(
		EnemyChaseState::Params(),
		*motion,
		*nav.grid,
		*nav.astar,
		[this]() { return trans.pos; },
		[this]() { return GetChaseGoal(); },
		[this]() { return ShouldChase(); },
		[this]() { AnimePlay(CHARACTER_ANIME::WALK); }
	);
	AddState(STATE::Chase, chaseState);

	// 「待機」->「追跡」(追跡する理由ができたら)
	RegisterStateTransition(STATE::Idle, STATE::Chase);
	// 「追跡」->「待機」(追跡する理由がなくなったら)
	RegisterStateTransition(STATE::Chase, STATE::Idle);

#pragma endregion
}

void Enemy::SubInit(void)
{
	trans.pos = spawnPos;
	trans.scale = Vector3(3.0f, 3.0f, 3.0f);

	// 最初は待機状態から始める
	ChangeState(STATE::Idle);
}

void Enemy::SubUpdate(void)
{
	// 状態の更新(BaseUpdate)より先に、目標が見えているかを更新しておく
	UpdatePerception();
}

bool Enemy::IsChasing(void)const
{
	return GetState() == static_cast<int>(STATE::Chase);
}

void Enemy::UpdatePerception(void)
{
	canSeeTarget = false;

	if (targetPos != nullptr && nav.grid != nullptr)
	{
		const Vector3 eye = trans.pos + Vector3(0.0f, EYE_HEIGHT, 0.0f);
		const Vector3 aim = *targetPos + Vector3(0.0f, TARGET_AIM_HEIGHT, 0.0f);
		const Vector3 toTarget = aim - eye;

		// 1) 距離(追跡中は少し遠くまで見える)
		const bool chasing = IsChasing();
		const float range = SIGHT_RANGE * (chasing ? CHASE_SIGHT_RANGE_RATE : 1.0f);
		bool inSight = toTarget.LengthSq() <= range * range;

		// 2) 視野角(追跡中は全方位)
		if (inSight && !chasing)
		{
			Vector3 forward = trans.rotation.Rotate(Vector3(0.0f, 0.0f, 1.0f));
			forward.y = 0.0f;
			Vector3 toXZ(toTarget.x, 0.0f, toTarget.z);

			if (forward.LengthSq() > 1.0e-6f && toXZ.LengthSq() > 1.0e-6f)
			{
				const float cosAngle = forward.Normalized().Dot(toXZ.Normalized());
				inSight = cosAngle >= std::cos(SIGHT_HALF_ANGLE_DEG * (DX_PI_F / 180.0f));
			}
		}

		// 3) 遮蔽(ビルなどの陰に入っていないか)
		canSeeTarget = inSight && nav.grid->HasLineOfSight(eye, aim);
	}

	if (canSeeTarget)
	{
		lastKnownPos = *targetPos;
		hasLastKnownPos = true;
		lostFrames = 0.0f;
	}
	else
	{
		lostFrames += TimeScale::Get();
	}
}

bool Enemy::ShouldChase(void)const
{
	if (canSeeTarget) { return true; }

	// 見失っても、最後に見た位置を一定時間は探す
	return hasLastKnownPos && lostFrames < LOSE_SIGHT_FRAMES;
}

Vector3 Enemy::GetChaseGoal(void)const
{
	if (canSeeTarget && targetPos != nullptr) { return *targetPos; }
	return lastKnownPos;
}

void Enemy::DrawDebug(void)const
{
	const Vector3 base = trans.pos;

	// 状態(待機=灰色、追跡=赤)を頭上の球で表す
	const unsigned int stateColor = IsChasing() ? 0xff3030 : 0x909090;
	DrawSphere3D((base + Vector3(0.0f, 280.0f, 0.0f)).ToVECTOR(), 18.0f, 8, stateColor, stateColor, TRUE);

	// 向いている方向の線(目標が見えていれば緑)
	Vector3 forward = trans.rotation.Rotate(Vector3(0.0f, 0.0f, 1.0f));
	forward.y = 0.0f;
	if (forward.LengthSq() > 1.0e-6f)
	{
		const Vector3 eye = base + Vector3(0.0f, EYE_HEIGHT, 0.0f);
		DrawLine3D(eye.ToVECTOR(), (eye + forward.Normalized() * 300.0f).ToVECTOR(), canSeeTarget ? 0x30ff30 : 0xffffff);
	}

	// 最後に見た位置
	if (hasLastKnownPos && !canSeeTarget && ShouldChase())
	{
		DrawSphere3D((lastKnownPos + Vector3(0.0f, 20.0f, 0.0f)).ToVECTOR(), 25.0f, 8, 0xffa000, 0xffa000, TRUE);
	}

	// 現在の経路(次に向かう点から先をシアンで)
	if (chaseState != nullptr)
	{
		const std::vector<PathPoint>& path = chaseState->GetPath();
		Vector3 prev = base;
		for (size_t i = chaseState->GetPathIndex(); i < path.size(); ++i)
		{
			DrawLine3D((prev + Vector3(0.0f, 25.0f, 0.0f)).ToVECTOR(), (path[i].pos + Vector3(0.0f, 25.0f, 0.0f)).ToVECTOR(), 0x00ffff);
			prev = path[i].pos;
		}
	}
}
