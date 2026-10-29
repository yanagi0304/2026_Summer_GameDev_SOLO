#include "Jazz.h"

#include "../../pch.h"

#include <iostream>

#include "../../Utility/Utility.h"

#include "../../Application/Application.h"

#include "../../Manager/Input/InputManager.h"

#include "../../Manager/Camera/CurrentCamera.h"

#include "../Common/Collider/SphereCollider.h"
#include "../Common/Collider/CapsuleCollider.h"

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

Jazz::Jazz(std::function<const Vector3& (void)> target) :
	CharacterBase("Data/Parameter/Character/Player/Wo-Chamolenium/"),

	weaponTrans(),

	mainCurrentIndex(0),
	subCurrentIndex(0),

	trigger(nullptr),

	target(target)
{
}

void Jazz::Load(void)
{

#pragma region オブジェクト設定

	// 動的オブジェクトとしての処理を有効にする
	SetDynamicFlg(true);

	// 重力を有効にする
	SetGravityFlg(true);

	// 当たり判定による押し出しを有効にする
	SetPushFlg(true);

	// 通常描画に設定
	SetDrawType(ACTOR_DRAW_TYPE::Alpha);
	
#pragma region モデル

	// モデルの読み込み
	trans.LoadModel("Character/Jazz/wocamore");

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
		[&]() { return GetAnimeRatio(); },
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
		[&]() { return GetAnimeRatio(); },
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
			10.0f, 10.0f, 30,
			std::bind(&Jazz::MoveAccel, this, std::placeholders::_1, true),
			ACCEL_MAX,
			[&]() { AnimePlay(CHARACTER_ANIME::WALK); },
			[&]() { AnimePlay(CHARACTER_ANIME::WALK); },
			[&]() {return GetIsGrashopper(); }
		)
	);

	// ジャンプ状態
	AddState(
		STATE::Jump,
		new PlayerJumpState(
			20.0f, velocity.y, isGround,
			std::bind(&Jazz::MoveAccel, this, std::placeholders::_1,true),
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A1); },
			[&]() { AnimePlay(CHARACTER_ANIME::ASTEROID_CREATE_R); },
			[&]() { AnimePlay(CHARACTER_ANIME::SLASH_A1); },
			std::bind(&Jazz::IsAnimeEnd, this),
			[&]() { ChangeState(STATE::Idle); }
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
	for (auto& w : mainWeaponSet) { w.second->SetIsActive(true); }
	WeaponChange(TriggerType::KOGETSU);
}

void Jazz::SubUpdate(void)
{
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMainSwitch).down && CanWeaponChange()) { WeaponChange(); }
}

bool Jazz::CanWeaponChange(void) const
{
	// 攻撃中などは不可。切り替えてよい状態だけを許可する(ホワイトリスト)
	const STATE s = static_cast<STATE>(GetState());
	return s == STATE::Idle || s == STATE::Move;
}

void Jazz::WeaponChange(TriggerType type)
{
	const auto& mains = trigger->GetMainTrigger();

	auto it = std::find(mains.begin(), mains.end(), type);

	if (it != mains.end()) { mainCurrentIndex = (int)(it - mains.begin()); }
	else { mainCurrentIndex = (mainCurrentIndex + 1) % (int)mains.size(); }

	currentWeaponID = mains[mainCurrentIndex];

	for (auto& w : mainWeaponSet) { w.second->SetIsActive(w.first == currentWeaponID); }

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

void Jazz::IsJump(void)
{
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerJump).down && isGround) {
		AnimePlay(CHARACTER_ANIME::JUMP);

		velocity.y = 20.0f;

	}
}

bool Jazz::IsTrigger(TriggerType trigger)
{
	return currentWeaponID == trigger;
}
