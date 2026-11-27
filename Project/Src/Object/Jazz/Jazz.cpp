#include "Jazz.h"

#include "../../pch.h"

#include <algorithm>
#include <iostream>

#include "../../Utility/Utility.h"

#include "../../Application/Application.h"

#include "../../Manager/Input/InputManager.h"

#include "../../Manager/Camera/CurrentCamera.h"

#include "../Common/Collider/SphereCollider.h"
#include "../Common/Collider/CapsuleCollider.h"

#include "../Common/PhysicsBody/PhysicsBody.h"

#include "../Common/MotionController/GroundMotionController.h"

#include "../Common/AnimationController/AnimationController.h"

#include "../TriggerUI/TriggerUI.h"

#include "../Weapon/WeaponBase.h"
#include "../Weapon/Kogetsu/Kogetsu.h"
#include "../Weapon/Asteroid/Asteroid.h"
#include "../Weapon/Manager/ShooterManager.h"
#include "../Weapon/Shield/Shield.h"
#include "../Weapon/Grasshopper/Grasshopper.h"
#include "../Weapon/Senku/Senku.h"

#include "../Common/Effect/PolyDecompose/PolyDecompose.h"

#include "../../Manager/TimeScale/TimeScale.h"

#include "State/PlayerIdleState.h"
#include "State/PlayerJumpState.h"
#include "State/PlayerGrasshopperJumpState.h"
#include "State/PlayerMoveState.h"
#include "State/PlayerKogetsuState.h"

#include "../../Manager/Camera/ShoulderCamera/ShoulderCamera.h"

Jazz::Jazz() :
	CharacterBase("Data/Parameter/Character/Player/Wo-Chamolenium/"),

	weaponTrans(),
	prevPos(),

	trigger(nullptr),


	isAim(false),

	spawnEffect(nullptr),
	spawnFrame(0.0f)
{
}

void Jazz::Load(void)
{

#pragma region オブジェクト設定

	// 動的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Movable);

	// 物理挙動の有効化
	EnablePhysics();

	// 重力を有効にする
	GetPhysicsBody()->SetGravityEnabled(true);
	// 弾性力の設定
	GetPhysicsBody()->SetRestitution(0.0f);


	// キャラクター用の物理挙動に設定する
	GetPhysicsBody()->SetMode(PhysicsBody::MotionMode::Character);

	// 当たり判定による押し出しを有効にする
	SetContactResponseEnabled(true);
	// 物理衝突時の重さを設定
	GetPhysicsBody()->SetMass(1.0f);

	// 移動・ジャンプ・回避で共有するControllerを生成
	GroundMotionController* groundMotionController = new GroundMotionController();

	// アクターにMotionControllerを登録
	AddMotionController(groundMotionController);

	// モデル制御情報を設定
	groundMotionController->SetTransform(&trans);
	// 接地判定の参照先を設定
	groundMotionController->SetGroundedSource(GroundedFlag());
	// 接地時の加速度を設定
	groundMotionController->SetGroundAcceleration(3.0f);
	// 空中時の加速度を設定
	groundMotionController->SetAirAcceleration(3.0f);
	// 慣性操作の強さを設定
	groundMotionController->SetMomentumSteering(0.2f);
	// 慣性操作が効く速度の閾値を設定
	groundMotionController->SetMomentumThreshold(40.0f);
	// ジャンプ力を設定
	groundMotionController->SetJumpSpeed(30.0f);

#pragma endregion

#pragma region モデル設定
	// モデルの読み込み
	trans.LoadModel("Character/WoChamolenium/WoChamolenium");

	trans.SetLocalRotation(Quaternion::FromRotationY(Deg2Rad(180.0f)));

#pragma endregion

#pragma region アニメーション読み込み

	// アニメーションコントローラー生成
	CreateAnimationController();
	// アニメーション登録
	AddInFbxAnimation(CHARACTER_ANIME::MAX, ANIME_SPEED_TABLE, ANIME_LOOP_TABLE);

	anime->SetPositionCorrectionFrame("mixamorig:Hips");

	anime->SetPositionCorrection(true);
	
#pragma endregion

#pragma region コライダー生成

	// モデルのコライダー生成
	AddCollider(
		new CapsuleCollider(
			COLLIDER_TAG::Player,
			CAPSULE_START,
			CAPSULE_END,
			CAPSULE_RADIUS
		)
	);

	// プレイヤーのアタックコライダーの生成
	CreateAttackCollider(
		COLLIDER_TAG::PlayerAttack,
		COLLIDER_TAG::Enemy,
		CAPSULE_RADIUS,
		Vector3(0.0f, 30.0f, 80.0f),
		10.0f
	);

#pragma endregion

#pragma region 下位アクター生成

	// トリガーUIの生成
	trigger = new TriggerUI(mainSlot.currentIndex, subSlot.currentIndex);

	// トリガーUI管理クラスをJazzクラスの下位クラスとして登録
	AddChildActor(trigger);

#pragma region 武器の生成

	//メイントリガー

	// 武器がトリオンを消費するときに呼ぶ関数
	const auto decreaseTrion = [this](float value) { DecreaseTrion(value); };

	// 「弧月」生成
	mainSlot.set.emplace(TriggerType::KOGETSU, new Kogetsu(trans, -1, decreaseTrion));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainSlot.set.at(TriggerType::KOGETSU));

	// 「アステロイド」生成
	mainSlot.set.emplace(TriggerType::ASTEROID, new ShooterManager(
		trans,
		ShooterConfig::MakeAsteroid(),
		[&]() { if (static_cast<STATE>(GetState()) == STATE::Idle) { AnimePlay(mainSlot.set.at(mainSlot.currentID)->GetIdleAnimeID()); } },
		decreaseTrion));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainSlot.set.at(TriggerType::ASTEROID));

	// 「シールド」生成
	mainSlot.set.emplace(TriggerType::SHIELD, new Shield(trans, decreaseTrion));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainSlot.set.at(TriggerType::SHIELD));

	// 「グラスホッパー」生成
	mainSlot.set.emplace(TriggerType::GRASS_HOPPER, new Grasshopper(
		trans,
		[&](bool isStraightUp) { OnGrasshopperLaunch(isStraightUp); },
		decreaseTrion));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainSlot.set.at(TriggerType::GRASS_HOPPER));

	// トリガーUIの登録
	for (auto& w : mainSlot.set) { trigger->AddMain(w.first); }

	//サブトリガー
	
	//「ハウンド」生成
	subSlot.set.emplace(TriggerType::HOUND, new ShooterManager(
		trans,
		ShooterConfig::MakeHound(
			[this]() { return lockOn.GetTargetId(); },
			[this](int id) { return lockOn.GetPos(id); }),
		[&]() { if (static_cast<STATE>(GetState()) == STATE::Idle) { AnimePlay(mainSlot.set.at(mainSlot.currentID)->GetIdleAnimeID()); } },
		decreaseTrion));

	// Jazzクラスの下位クラスとして登録する
	AddChildActor(subSlot.set.at(TriggerType::HOUND));

	//「旋空」生成
	subSlot.set.emplace(TriggerType::SENKU, new Senku(trans, decreaseTrion));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(subSlot.set.at(TriggerType::SENKU));

	for (auto& w : subSlot.set) { trigger->AddSub(w.first); }

#pragma endregion

#pragma endregion

#pragma region 状態設定

	// 待機状態
	AddState(
		STATE::Idle,
		new PlayerIdleState([&]() { AnimePlay(mainSlot.set.at(mainSlot.currentID)->GetIdleAnimeID()); })
	);

	// 移動状態
	AddState(
		STATE::Move,
		new PlayerMoveState(
			10.0f, 2.0f, 500,
			*groundMotionController,
			[&]() { AnimePlay(CHARACTER_ANIME::WALK); },
			[&]() { AnimePlay(CHARACTER_ANIME::NinjaRun); }
		)
	);

	// ジャンプ状態
	AddState(
		STATE::Jump,
		new PlayerJumpState(
			*groundMotionController,
			[&]() { AnimePlay(CHARACTER_ANIME::JUMP_START); },
			[&]() { AnimePlay(CHARACTER_ANIME::JUMPING); },
			[&]() { AnimePlay(CHARACTER_ANIME::JUMP_END); },
			std::bind(&Jazz::IsAnimeEnd, this),
			[&]() { ChangeState(STATE::Idle); },
			[&]() {return GetAnimeRatio(); }
		)
	);

	// グラスホッパーで打ち上げられた状態
	AddState(
		STATE::GrasshopperJump,
		new PlayerGrasshopperJumpState(
			*groundMotionController,
			[&]() { AnimePlay(CHARACTER_ANIME::JUMP_START); },
			[&]() { AnimePlay(CHARACTER_ANIME::JUMPING); },
			[&]() { AnimePlay(CHARACTER_ANIME::JUMP_END); },
			std::bind(&Jazz::IsAnimeEnd, this),
			[&]() { PhysicsBody* body = GetPhysicsBody(); return (body != nullptr) ? body->GetVelocity().y : 0.0f; },
			[&]() { ChangeState(STATE::Idle); },
			[&]() { ChangeState(STATE::Jump); }
		)
	);
	//弧月
	AddState(
		STATE::Kogetsu,
		new PlayerKogetsuState(
			{
				0.1f,
				0.3f,
				0.5f
			},
			{
				0.7f,
				0.7f,
				0.9f
			},
			*mainSlot.set.at(TriggerType::KOGETSU),
			[&]() { return IsTrigger(TriggerType::KOGETSU); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A1); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A2); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A3); },
			[&]() {return GetAnimeRatio(); },
			[&]() {ChangeState(STATE::Idle); },
			[&]() {AttackOn(); },
			[&]() {AttackOff(); }
		)
	);

	// 「待機状態」->「移動状態」の自動遷移登録
	RegisterStateTransition(STATE::Idle, STATE::Move);
	// 「移動状態」->「待機状態」の自動遷移登録
	RegisterStateTransition(STATE::Move, STATE::Idle);

	// 「待機状態」->「移動状態」の自動遷移登録
	RegisterStateTransition(STATE::Idle, STATE::Jump);
	// 「移動状態」->「待機状態」の自動遷移登録
	RegisterStateTransition(STATE::Move, STATE::Jump);

	//「待機状態」->「弧月」の自動遷移登録
	RegisterStateTransition(STATE::Idle, STATE::Kogetsu);
	// 「移動状態」->「弧月」の自動遷移登録
	RegisterStateTransition(STATE::Move, STATE::Kogetsu);

	//「待機状態」->「グラスホッパー」の自動遷移登録
	RegisterStateTransition(STATE::Idle, STATE::GrasshopperJump);
	// 「移動状態」->「グラスホッパー」の自動遷移登録
	RegisterStateTransition(STATE::Move, STATE::GrasshopperJump);

#pragma endregion
}

void Jazz::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	switch (other.GetTag())
	{
	case COLLIDER_TAG::Grasshopper:
		
		break;
	default:
		break;
	}
}

void Jazz::SubInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = GetParameterToVector3("Init", "Scale");
	// サブ武器は、UIの先頭に表示しているハウンドを装備して始める
	// (待機アニメはメイン武器に合わせるので、アニメ再生より先に行う)
	SwitchWeapon(subSlot, trigger->GetSubTrigger(), TriggerType::HOUND);

	AnimePlay(CHARACTER_ANIME::IDLE);
//	for (auto& w : mainSlot.set) { w.second->SetIsActive(false); }
	WeaponChange(TriggerType::KOGETSU);

	// 生成演出の準備
	if (spawnParams.frames > 0.0f)
	{
		trans.Attach();
		spawnEffect = new PolyDecompose();
		if (!spawnEffect->Init(trans.model))
		{
			// 準備できなければ演出なしで出す
			spawnEffect->Release();
			delete spawnEffect;
			spawnEffect = nullptr;
		}
	}
}

void Jazz::SubUpdate(void)
{
	// 生成演出を進める。終わったら通常の描画に切り替える
	if (spawnEffect != nullptr)
	{
		spawnFrame += TimeScale::Get();
		if (spawnFrame >= spawnParams.frames)
		{
			spawnEffect->Release();
			delete spawnEffect;
			spawnEffect = nullptr;
		}
	}

	//エイム中か
	ShoulderCamera* cam = dynamic_cast<ShoulderCamera*>(&CurrentCamera::Get());
	if (cam != nullptr) { isAim = cam->IsAiming(); }

	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMainSwitch).down && CanWeaponChange()) { SwitchWeapon(mainSlot, trigger->GetMainTrigger()); }
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerSubSwitch).down && CanWeaponChange()) { SwitchWeapon(subSlot, trigger->GetSubTrigger()); }

	// 弧月を手に持つか腰に付けるかを更新
	UpdateKogetsuSheath();

	if (isAim) { AimRotate(); }

	// ハウンドを装備している間だけロックオンする
	lockOn.Update(subSlot.currentID == TriggerType::HOUND);
}

void Jazz::OnGrasshopperLaunch(bool isStraightUp)
{
	// 攻撃中などは割り込まない
	const STATE s = static_cast<STATE>(GetState());
	if (s == STATE::Idle || s == STATE::Move || s == STATE::Jump || s == STATE::GrasshopperJump) {
		// 真上なら最初から移動できる(状態に入る前に設定する)
		static_cast<PlayerGrasshopperJumpState&>(GetStateIns(STATE::GrasshopperJump)).SetControllableFromStart(isStraightUp);
		ChangeState(STATE::GrasshopperJump);
	}
}
bool Jazz::CanWeaponChange(void) const
{
	// 攻撃中などは不可。切り替えてよい状態だけを許可する
	const STATE s = static_cast<STATE>(GetState());
	return s == STATE::Idle || s == STATE::Move || s == STATE::Jump || s == STATE::GrasshopperJump;
}

void Jazz::UpdateKogetsuSheath(void)
{
	Kogetsu* kogetsu = dynamic_cast<Kogetsu*>(mainSlot.set.at(TriggerType::KOGETSU));
	if (kogetsu == nullptr) { return; }

	// Idle と Kogetsu(攻撃)ステートのときだけ手に持つ
	const STATE s = static_cast<STATE>(GetState());
	kogetsu->SetSheathed(!(s == STATE::Kogetsu));
}

void Jazz::WeaponChange(TriggerType type)
{
	const auto& mains = trigger->GetMainTrigger();

	if (mains.empty()) { return; }

	if (type == TriggerType::NONE) {
		mainSlot.currentIndex = (mainSlot.currentIndex + 1) % (int)mains.size();
	}
	else if (auto it = std::find(mains.begin(), mains.end(), type); it != mains.end()) {
		mainSlot.currentIndex = (int)(it - mains.begin());
	}
	else {
		return;   // 未登録の武器は無視する
	}

	// 同じ武器なら何もしない
	const TriggerType next = mains[mainSlot.currentIndex];
	if (next == mainSlot.currentID) { return; }

	// 前の武器だけ止める
	if (auto prev = mainSlot.set.find(mainSlot.currentID); prev != mainSlot.set.end()) {
		prev->second->SetIsActive(false);
	}

	// 新しい武器だけ起動する
	mainSlot.currentID = next;
	mainSlot.set.at(mainSlot.currentID)->SetIsActive(true);

	// Idle中のみ即座に反映。Move/Jump中は上書きしない（Idle復帰時に現武器のIdleが再生される）
	if (static_cast<STATE>(GetState()) == STATE::Idle) {
		AnimePlay(mainSlot.set.at(mainSlot.currentID)->GetIdleAnimeID());
	}
}

void Jazz::SwitchWeapon(WeaponSlot& slot, const std::vector<TriggerType>& triggers, TriggerType type)
{

	if (triggers.empty()) { return; }

	if (type == TriggerType::NONE) {
		slot.currentIndex = (slot.currentIndex + 1) % (int)triggers.size();
	}
	else if (auto it = std::find(triggers.begin(), triggers.end(), type); it != triggers.end()) {
		slot.currentIndex = (int)(it - triggers.begin());
	}
	else {
		return;   // 未登録の武器は無視する
	}

	// 同じ武器なら何もしない
	const TriggerType next = triggers[slot.currentIndex];
	if (next == slot.currentID) { return; }

	// 前の武器だけ止める
	if (auto prev = slot.set.find(slot.currentID); prev != slot.set.end()) {
		prev->second->SetIsActive(false);
	}

	// 新しい武器だけ起動する
	slot.currentID = next;
	slot.set.at(slot.currentID)->SetIsActive(true);

	// Idle中のみ即座に反映。Move/Jump中は上書きしない（Idle復帰時に現武器のIdleが再生される）
	if (static_cast<STATE>(GetState()) == STATE::Idle) {
		AnimePlay(slot.set.at(slot.currentID)->GetIdleAnimeID());
	}
}

Vector3 Jazz::InputVec(void) const
{
	// 返却用一時変数
	Vector3 vec = Vector3();

	// コントローラーの入力を取得
	vec = Input::GetIns().GetLeftStickVec(false).ToVector3XZInvertY();

	// 入力がなければ次にキーボードの入力を取得
	if (vec == 0.0f) {
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveRight).now) { vec.x++; }
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveLeft).now) { vec.x--; }
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveFront).now) { vec.z++; }
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveBack).now) { vec.z--; }
		vec.Normalize();
	}

	return vec;
	
}

void Jazz::SubDraw(void)
{
	// 【仮】残りトリオンの描画
	const int gaugeX = Application ::SCREEN_SIZE_X_HALF;
	const int gaugeY = Application::SCREEN_SIZE_Y - 60;
	const int gaugeW = 360;
	const int gaugeH = 22;
	const float trionRatio = std::clamp(GetTrionRatio(), 0.0f, 1.0f);
	DrawBox(gaugeX - 3, gaugeY - 3, gaugeX + gaugeW + 3, gaugeY + gaugeH + 3, 0x000000, TRUE);
	DrawBox(gaugeX, gaugeY, gaugeX + static_cast<int>(gaugeW * trionRatio), gaugeY + gaugeH, 0x30c0ff, TRUE);
	DrawBox(gaugeX - 3, gaugeY - 3, gaugeX + gaugeW + 3, gaugeY + gaugeH + 3, 0xffffff, FALSE);
	DrawFormatString(gaugeX, gaugeY - 22, 0xffffff, "TRION  %.0f%%", trionRatio * 100.0f);

	// ロックオン中の目標の印
	lockOn.DrawMarker();

	//DrawFormatString(0, 120, 0x000000, "%f,%f,%f", trans.pos.x, trans.pos.y, trans.pos.z);
}

bool Jazz::IsTrigger(TriggerType trigger)
{
	return mainSlot.currentID == trigger;
}

void Jazz::AimRotate(void)
{
	// カメラの水平方向の向き
	const float camYaw = CurrentCamera::Get().GetRotation().GetYaw();

	// 現在の回転から目標の回転へ補間する
	trans.SetRotation(
		Quaternion::Slerp(
			trans.rotation,
			Quaternion::FromRotationY(camYaw),
			TimeScale::Get()
		)
	);
}

void Jazz::Draw(void)
{
	// 通常は、そのままモデルを描く
	if (spawnEffect == nullptr)
	{
		CharacterBase::Draw();
		return;
	}

	if (!GetIsDraw()) { return; }

	// トリオンゲージなどは、通常の描画と同じように出す
	SubDraw();

	// 生成演出中は、三角形の破片が足元から組み上がる(進行度は1→0)
	trans.Attach();
	spawnEffect->Update();

	spawnEffect->Draw(1.0f - spawnFrame / spawnParams.frames, spawnParams);
}

void Jazz::SubRelease(void)
{
	if (spawnEffect != nullptr)
	{
		spawnEffect->Release();
		delete spawnEffect;
		spawnEffect = nullptr;
	}
}