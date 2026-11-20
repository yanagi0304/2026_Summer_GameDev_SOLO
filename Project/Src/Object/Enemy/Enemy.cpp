#include "Enemy.h"

#include <chrono>
#include <cmath>

#include "../../pch.h"

#include "../../Utility/Utility.h"

#include "../../Manager/TimeScale/TimeScale.h"

#include "../Common/Collider/CapsuleCollider.h"
#include "../Common/PhysicsBody/PhysicsBody.h"
#include "../Common/MotionController/GroundMotionController.h"
#include "../Common/AnimationController/AnimationController.h"

#include "Navigation/NavGrid.h"
#include "Navigation/NavManager.h"
#include "State/EnemyIdleState.h"
#include "State/EnemyChaseState.h"
#include "State/EnemyAttackState.h"

#include "../Weapon/Kogetsu/Kogetsu.h"

Enemy::Enemy(const NavContext& nav, const Vector3* targetPos, const Vector3& spawnPos, const std::vector<Enemy*>* peers, int baseModel, const Combat& combat) :
	CharacterBase("Data/Parameter/Character/Rasenoniumu/"),

	nav(nav),
	targetPos(targetPos),
	spawnPos(spawnPos),
	peers(peers),
	baseModel(baseModel),
	combat(combat),
	defeated(false),

	chaseState(nullptr),

	canSeeTarget(false),
	hasLastKnownPos(false),
	lastKnownPos(),
	lostFrames(0.0f),

	kogetsu(nullptr),
	attackCooldown(0.0f)
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
	motion->SetGroundedSource(GroundedFlag());
	motion->SetGroundAcceleration(3.0f);
	motion->SetAirAcceleration(3.0f);
	motion->SetMomentumSteering(0.2f);
	motion->SetMomentumThreshold(40.0f);

#pragma endregion

#pragma region モデル設定

	// 仮のモデル(プレイヤーと同じ)。複製元があれば複製する(ファイルからの読み込みは重い)
	const auto modelStart = std::chrono::high_resolution_clock::now();
	if (baseModel >= 0) { ModelDuplicate(baseModel); }
	else { trans.LoadModel("Character/Rasenoniumu/Rasenoniumu"); }
	lastModelLoadMs = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - modelStart).count();
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
			GetParameterToVector3("Collider", "ColliderStartPos"),
			GetParameterToVector3("Collider", "ColliderEndPos"),
			GetParameter("Collider", "Radius")
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
		*nav.manager,
		[this]() { return trans.pos; },
		[this]() { return GetChaseGoal(); },
		[this]() { return ShouldChase(); },
		[this]() { return ComputeSeparation(); },
		[this]() { AnimePlay(CHARACTER_ANIME::WALK); }
	);
	AddState(STATE::Chase, chaseState);

	// 弧月を持たせる(右手のボーンに追従する)。
	// 弧月に付いているコライダーはメッシュカット専用なので、攻撃の判定には使わない(On/Offも呼ばない)
	kogetsu = new Kogetsu(trans, combat.swordModel);
	AddChildActor(kogetsu);
	kogetsu->SetIsActive(true);

	// 攻撃状態
	EnemyAttackState::Callbacks attack;
	attack.canStart = [this]() { return CanStartAttack(); };
	attack.getTargetDirection = [this]() { return targetPos != nullptr ? (*targetPos - trans.pos) : Vector3(0.0f, 0.0f, 0.0f); };
	attack.playAnimeAttack = [this]() { AnimePlay(CHARACTER_ANIME::SLASH_A1); };
	attack.getAnimeRatio = [this]() { return GetAnimeRatio(); };
	attack.isAnimeEnd = [this]() { return IsAnimeEnd(); };
	attack.onFinished = [this]()
	{
		attackCooldown = ATTACK_COOLDOWN_FRAMES;
		ChangeState(ShouldChase() ? STATE::Chase : STATE::Idle);
	};
	AddState(STATE::Attack, new EnemyAttackState(EnemyAttackState::Params(), *motion, attack));

	// 「待機」->「追跡」(追跡する理由ができたら)
	RegisterStateTransition(STATE::Idle, STATE::Chase);
	// 「追跡」->「待機」(追跡する理由がなくなったら)
	RegisterStateTransition(STATE::Chase, STATE::Idle);
	// 攻撃できる状況になったら攻撃へ(攻撃が終わったら、状態の中から次の状態へ移る)
	RegisterStateTransition(STATE::Idle, STATE::Attack);
	RegisterStateTransition(STATE::Chase, STATE::Attack);

#pragma endregion
}

void Enemy::SubInit(void)
{
	trans.pos = spawnPos;
	trans.scale = Vector3(SCALE);

	// 最初は待機状態から始める
	ChangeState(STATE::Idle);
}

void Enemy::SubUpdate(void)
{
	// 状態の更新(BaseUpdate)より先に、目標が見えているかを更新しておく
	UpdatePerception();

	if (attackCooldown > 0.0f) { attackCooldown -= TimeScale::Get(); }
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
		const bool chasing = IsChasing() || GetState() == static_cast<int>(STATE::Attack);
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

bool Enemy::CanStartAttack(void)const
{
	if (defeated || attackCooldown > 0.0f || !canSeeTarget || targetPos == nullptr) { return false; }

	const float dx = targetPos->x - trans.pos.x;
	const float dz = targetPos->z - trans.pos.z;
	if (dx * dx + dz * dz > ATTACK_RANGE * ATTACK_RANGE) { return false; }

	return std::abs(targetPos->y - trans.pos.y) <= ATTACK_HEIGHT_TOLERANCE;
}

Vector3 Enemy::GetChaseGoal(void)const
{
	if (canSeeTarget && targetPos != nullptr) { return *targetPos; }
	return lastKnownPos;
}

Vector3 Enemy::ComputeSeparation(void)const
{
	Vector3 push(0.0f, 0.0f, 0.0f);
	if (peers == nullptr) { return push; }

	for (const Enemy* other : *peers)
	{
		if (other == this || other->IsDefeated()) { continue; }

		// 水平方向だけで見る
		const float dx = trans.pos.x - other->trans.pos.x;
		const float dz = trans.pos.z - other->trans.pos.z;
		const float dist = std::sqrt(dx * dx + dz * dz);
		if (dist >= SEPARATION_RADIUS) { continue; }

		// 近いほど強く、相手から遠ざかる向きへ
		const float strength = (SEPARATION_RADIUS - dist) / SEPARATION_RADIUS;
		if (dist > 1.0e-3f)
		{
			push.x += dx / dist * strength;
			push.z += dz / dist * strength;
		}
		else
		{
			// 完全に重なっている時は、向きが決まらないので適当な方向へ
			push.x += strength;
		}
	}

	// 長さは最大1にする
	const float length = push.Length();
	if (length > 1.0f) { push = push / length; }
	return push;
}

void Enemy::DrawDebug(void)const
{
	const Vector3 base = trans.pos;

	// 状態(待機=灰色、追跡=赤)を頭上の球で表す
	const unsigned int stateColor = GetState() == static_cast<int>(STATE::Attack) ? 0xffff30 : (IsChasing() ? 0xff3030 : 0x909090);
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

void Enemy::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	if (other.GetTag() != COLLIDER_TAG::Asteroid) { return; }

	Defeat();

}
