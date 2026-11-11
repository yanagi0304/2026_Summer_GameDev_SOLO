#include "Jazz.h"

#include "../../pch.h"

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
#include "../../Manager/TimeScale/TimeScale.h"

#include "State/PlayerIdleState.h"
#include "State/PlayerJumpState.h"
#include "State/PlayerMoveState.h"
#include "State/PlayerKogetsuState.h"

#include "../Common/Target/Target.h"
#include "../../Manager/Camera/ShoulderCamera/ShoulderCamera.h"

Jazz::Jazz() :
	CharacterBase("Data/Parameter/Character/Player/Wo-Chamolenium/"),

	weaponTrans(),
	prevPos(),

	mainCurrentIndex(0),
	subCurrentIndex(0),

	trigger(nullptr),

	isGrashpper(false),

	currentWeaponID(TriggerType::NONE),

	isAim(false)
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
	groundMotionController->SetGroundedSource(&isGround);
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
			GetParameterToVector3("Collider", "ColliderStartPos"),
			GetParameterToVector3("Collider", "ColliderEndPos"),
			GetParameter("Collider", "Radius")
		)
	);
#pragma endregion

#pragma region 下位アクター生成

	// トリガーUIの生成
	trigger = new TriggerUI(mainCurrentIndex, subCurrentIndex);

	// トリガーUI管理クラスをJazzクラスの下位クラスとして登録
	AddChildActor(trigger);

#pragma region 武器の生成

	//メイントリガー

	// 「弧月」生成
	mainWeaponSet.emplace(TriggerType::KOGETSU, new Kogetsu(trans));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainWeaponSet.at(TriggerType::KOGETSU));

	// 「アステロイド」生成
	mainWeaponSet.emplace(TriggerType::ASTEROID, new ShooterManager(
		trans,
		ShooterConfig::MakeAsteroid(),
		[&]() {AnimePlay(mainWeaponSet.at(currentWeaponID)->GetIdleAnimeID()); }));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainWeaponSet.at(TriggerType::ASTEROID));

	// 「シールド」生成
	mainWeaponSet.emplace(TriggerType::SHIELD, new Shield(trans));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainWeaponSet.at(TriggerType::SHIELD));

	// 「グラスホッパー」生成
	mainWeaponSet.emplace(TriggerType::GRASS_HOPPER, new Grasshopper(
		trans,isGrashpper));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(mainWeaponSet.at(TriggerType::GRASS_HOPPER));

	// トリガーUIの登録
	for (auto& w : mainWeaponSet) { trigger->AddMain(w.first); }

	//サブトリガー
	// 「ハウンド」生成
	subWeaponSet.emplace(TriggerType::HOUND, new ShooterManager(
		trans,
		ShooterConfig::MakeHound(),
		[&]() {AnimePlay(mainWeaponSet.at(currentWeaponID)->GetIdleAnimeID()); }));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(subWeaponSet.at(TriggerType::HOUND));

	for (auto& w : subWeaponSet) { trigger->AddSub(w.first); }

#pragma endregion

#pragma endregion

#pragma region 状態設定

	// 待機状態
	AddState(
		STATE::Idle,
		new PlayerIdleState([&]() { AnimePlay(mainWeaponSet.at(currentWeaponID)->GetIdleAnimeID()); })
	);

	// 移動状態
	AddState(
		STATE::Move,
		new PlayerMoveState(
			10.0f, 2.0f, 300,
			*groundMotionController,
			[&]() { AnimePlay(CHARACTER_ANIME::WALK); },
			[&]() { AnimePlay(CHARACTER_ANIME::RUN); }
		)
	);

	// ジャンプ状態
	AddState(
		STATE::Jump,
		new PlayerJumpState(
			*groundMotionController,isGrashpper,
			[&]() { AnimePlay(CHARACTER_ANIME::JUMP_START); },
			[&]() { AnimePlay(CHARACTER_ANIME::JUMPING); },
			[&]() { AnimePlay(CHARACTER_ANIME::JUMP_END); },
			std::bind(&Jazz::IsAnimeEnd, this),
			[&]() { ChangeState(STATE::Idle); },
			[&]() {return GetAnimeRatio(); }
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
			*mainWeaponSet.at(TriggerType::KOGETSU),
			[&]() { return IsTrigger(TriggerType::KOGETSU); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A1); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A2); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A3); },
			[&]() {return GetAnimeRatio(); },
			[&]() {ChangeState(STATE::Idle); }
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
	AnimePlay(CHARACTER_ANIME::IDLE);
//	for (auto& w : mainWeaponSet) { w.second->SetIsActive(false); }
	WeaponChange(TriggerType::KOGETSU);
}

void Jazz::SubUpdate(void)
{
	//エイム中か
	ShoulderCamera* cam = dynamic_cast<ShoulderCamera*>(&CurrentCamera::Get());
	if (cam != nullptr) { isAim = cam->IsAiming(); }

	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMainSwitch).down && CanWeaponChange()) { WeaponChange(); }

	// 弧月を手に持つか腰に付けるかを更新
	UpdateKogetsuSheath();

	if (isAim) { AimRotate(); }
}

bool Jazz::CanWeaponChange(void) const
{
	// 攻撃中などは不可。切り替えてよい状態だけを許可する
	const STATE s = static_cast<STATE>(GetState());
	return s == STATE::Idle || s == STATE::Move || s == STATE::Jump;
}

void Jazz::UpdateKogetsuSheath(void)
{
	Kogetsu* kogetsu = dynamic_cast<Kogetsu*>(mainWeaponSet.at(TriggerType::KOGETSU));
	if (kogetsu == nullptr) { return; }

	// Idle と Kogetsu(攻撃)ステートのときだけ手に持つ
	const STATE s = static_cast<STATE>(GetState());
	kogetsu->SetSheathed(!(s == STATE::Idle || s == STATE::Kogetsu));
}

void Jazz::WeaponChange(TriggerType type)
{
	const auto& mains = trigger->GetMainTrigger();

	if (mains.empty()) { return; }

	if (type == TriggerType::NONE) {
		mainCurrentIndex = (mainCurrentIndex + 1) % (int)mains.size();
	}
	else if (auto it = std::find(mains.begin(), mains.end(), type); it != mains.end()) {
		mainCurrentIndex = (int)(it - mains.begin());
	}
	else {
		return;   // 未登録の武器は無視する
	}

	// 同じ武器なら何もしない
	const TriggerType next = mains[mainCurrentIndex];
	if (next == currentWeaponID) { return; }

	// 前の武器だけ止める
	if (auto prev = mainWeaponSet.find(currentWeaponID); prev != mainWeaponSet.end()) {
		prev->second->SetIsActive(false);
	}

	// 新しい武器だけ起動する
	currentWeaponID = next;
	mainWeaponSet.at(currentWeaponID)->SetIsActive(true);

	AnimePlay(mainWeaponSet.at(currentWeaponID)->GetIdleAnimeID());
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
	//DrawFormatString(0, 120, 0x000000, "%f,%f,%f", trans.pos.x, trans.pos.y, trans.pos.z);
}

bool Jazz::IsTrigger(TriggerType trigger)
{
	return currentWeaponID == trigger;
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
