#include "ShooterManager.h"

#include "../../../Manager/Input/InputManager.h"

#include "../../../Manager/Camera/CurrentCamera.h"
#include "../../../Manager/Camera/ShoulderCamera/ShoulderCamera.h"

#include "../Asteroid/Asteroid.h"

#include "State/ShooterIdleState.h"
#include "State/ShooterCubeState.h"
#include "State/ShooterSplitState.h"
#include "State/ShooterFireState.h"

ShooterManager::ShooterManager(
	const Transform& ownerTrans,
	const ShooterConfig& config,
	const std::function<void(void)>& playAnime
) :
	WeaponBase(ownerTrans),

	config_(config),
	playAnime(playAnime),

	swarm_(),
	crosshair_(),

	states_(),
	currentState_(SHOOTER_STATE::Idle)
{
}

void ShooterManager::Load(void)
{
	// 弾を27発作って、群れにまとめる
	for (int i = 0; i < AsteroidSwarm::BULLET_NUM; i++)
	{
		Asteroid* bullet = new Asteroid(ownerTrans, config_);

		AddChildActor(bullet);

		bullet->Disappear();

		swarm_.Add(bullet);
	}

	// 各状態を作る
	states_.emplace(
		SHOOTER_STATE::Idle,
		new ShooterIdleState(swarm_)
	);

	states_.emplace(
		SHOOTER_STATE::Cube,
		new ShooterCubeState(
			swarm_,
			[this]() { return GetCenterPos(); }
		)
	);

	states_.emplace(
		SHOOTER_STATE::Split,
		new ShooterSplitState(
			ownerTrans,
			swarm_,
			[this]() { return GetCenterPos(); },
			[this](bool aim) { SetAiming(aim); }
		)
	);

	states_.emplace(
		SHOOTER_STATE::Fire,
		new ShooterFireState(ownerTrans, swarm_,
			[this]() {return CalcAimDir(); })
	);

	currentState_ = SHOOTER_STATE::Idle;
}

std::optional<Vector3> ShooterManager::CalcAimDir(void) const
{
	// ズームキーを押している間だけ
	if (!Input::GetIns().GetInfo(KEY_TYPE::CameraZoomIn).now) { return std::nullopt; }

	//ショルダーカメラの時だけ
	const ShoulderCamera* cam = dynamic_cast<ShoulderCamera*>(&CurrentCamera::Get());
	if (cam == nullptr) { return std::nullopt; }

	//カメラが向いている方向
	const Vector3 camForward = cam->GetRotation().Rotate(Vector3(0.0f, 0.0f, 1.0f));

	//画面中央を狙う
	const Vector3 aimPoint = cam->GetPos() + camForward * AIM_TARGET_DISTANCE;
	const Vector3 dir = aimPoint - GetCenterPos();

	if (dir.LengthSq() < 1.0e-6f) { return std::nullopt; }

	return dir.Normalized();
}

int ShooterManager::GetIdleAnimeID(void) const
{
	// 状態を作る前は、待機のアニメーションを返す
	const auto it = states_.find(currentState_);
	if (it == states_.end()) { return CHARACTER_ANIME::ASTEROID_IDLE_R; }

	return it->second->GetIdleAnimeID();
}

void ShooterManager::OnEquip(void)
{
	// 待機状態から始める
	if (!states_.empty()) { ChangeState(SHOOTER_STATE::Idle); }
}

void ShooterManager::OnUnequip(void)
{
	// 弾を消して、待機状態に戻す
	if (!states_.empty()) { ChangeState(SHOOTER_STATE::Idle); }
}

void ShooterManager::SubUpdate(void)
{
	// 現在の状態を更新する
	states_.at(currentState_)->Update();

	// ボタンで次の状態へ進める
	if (Input::GetIns().GetInfo(config_.advanceKey).down)
	{
		ChangeState(states_.at(currentState_)->NextState());

		// 新しい状態に合ったアニメーションを再生する
		if (playAnime) { playAnime(); }
	}
}

void ShooterManager::SubDraw(void)
{
	crosshair_.Draw();
}

void ShooterManager::SubRelease(void)
{
	// 状態を解放する
	for (auto& state : states_)
	{
		delete state.second;
		state.second = nullptr;
	}

	states_.clear();
}

void ShooterManager::ChangeState(SHOOTER_STATE next)
{
	// 今の状態の終了処理
	const auto current = states_.find(currentState_);
	if (current != states_.end()) { current->second->Exit(); }

	currentState_ = next;

	// 次の状態の初期化処理
	states_.at(currentState_)->Enter();
}

Vector3 ShooterManager::GetHandPos(void) const
{
	MV1SetMatrix(ownerTrans.model, ownerTrans.GetWorldMatrix());

	const int frameIndex = MV1SearchFrame(ownerTrans.model, "mixamorig:RightHandIndex2");

	// ボーンが見つからないときは、持ち主の座標から手の位置あたりを代用する
	if (frameIndex < 0) { return ownerTrans.pos + ownerTrans.rotation.Rotate(Vector3(80.0f, 160.0f, 0.0f)); }

	return Vector3(MV1GetFramePosition(ownerTrans.model, frameIndex));
}

// アステロイド群の中心位置を取得する
Vector3 ShooterManager::GetCenterPos(void) const
{
	const Vector3 right = ownerTrans.VTrans(Vector3(1.0f, 0.0f, 0.0f));

	return GetHandPos() + right * CENTER_OFFSET_RIGHT + Vector3(0.0f, CENTER_OFFSET_UP, 0.0f);
}

void ShooterManager::SetAiming(bool aim) const
{
	// ショルダーカメラのときだけ、ズームイン・ズームアウトさせる
	ShoulderCamera* cam = dynamic_cast<ShoulderCamera*>(&CurrentCamera::Get());
	if (cam != nullptr) { cam->SetAiming(aim); }
}
