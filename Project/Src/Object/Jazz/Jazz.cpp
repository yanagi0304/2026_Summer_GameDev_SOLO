#include "Jazz.h"

#include "../../pch.h"

#include <iostream>

#include "../../Application/Application.h"

#include "../../Manager/Input/InputManager.h"

#include "../../Manager/Camera/CurrentCamera.h"

#include "../Common/Collider/SphereCollider.h"
#include "../Common/Collider/CapsuleCollider.h"

#include "../Weapon/WeaponBase.h"
#include "../Weapon/Kogetsu/Kogetsu.h"
#include "../Weapon/Asteroid/Asteroid.h"
#include "../Weapon/Manager/ShooterManager.h"
#include "../Weapon/Shield/Shield.h"
#include "../Weapon/Grasshopper/Grasshopper.h"

Jazz::Jazz() :
	CharacterBase("Data/Parameter/Character/Player/Wo-Chamolenium/"),

	weaponTrans_(),

	mainCurrentIndex_(0),

	trigger_(mainCurrentIndex_, subCurrentIndex_)
{
}

void Jazz::Load(void)
{
	// アクターの挙動設定
	SetJudgeFlg(true);
	SetDynamicFlg(true);
	SetGravityFlg(true);

	// 通常描画に設定
	SetDrawType(ACTOR_DRAW_TYPE::Default);
	
#pragma region モデル

	// モデルの読み込み
	trans.LoadModel("Character/Jazz/Wochamole");

	// アニメーションコントローラー生成
	CreateAnimationController();
	// アニメーション登録
	AddInFbxAnimation(CHARACTER_ANIME::MAX, 0.5f);
	
#pragma endregion

	// モデルのコライダー生成
	AddCollider(
		new CapsuleCollider(
			COLLIDER_TAG::Player,
			GetParameterToVector3("Collider", "ColliderStartPos"),
			GetParameterToVector3("Collider", "ColliderEndPos"),
			GetParameter("Collider", "Radius")
		)
	);

	// トリガーUI管理クラスをJazzクラスの下位クラスとして登録
	AddChildActor(&trigger_);

#pragma region 武器の生成

	// 「弧月」生成
	weaponSet_.emplace(TriggerUI::TriggerType::KOGETSU, new Kogetsu(trans));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(weaponSet_.at(TriggerUI::TriggerType::KOGETSU));

	// 「アステロイド」生成
	weaponSet_.emplace(TriggerUI::TriggerType::ASTEROID, new ShooterManager(trans));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(weaponSet_.at(TriggerUI::TriggerType::ASTEROID));

	// 「シールド」生成
	weaponSet_.emplace(TriggerUI::TriggerType::SHIELD, new Shield(trans));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(weaponSet_.at(TriggerUI::TriggerType::SHIELD));

	// 「グラスホッパー」生成
	weaponSet_.emplace(TriggerUI::TriggerType::GRASS_HOPPER, new Grasshopper(trans));
	// Jazzクラスの下位クラスとして登録する
	AddChildActor(weaponSet_.at(TriggerUI::TriggerType::GRASS_HOPPER));

	// トリガーUIの登録
	for (auto& w : weaponSet_) { trigger_.AddMain(w.first); }

#pragma endregion

	// 現在選択中の武器を「弧月」に設定
	currentWeaponID_ = TriggerUI::TriggerType::KOGETSU;
}

void Jazz::SubInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = GetParameterToVector3("Init", "Scale");
	AnimePlay(CHARACTER_ANIME::SWORD_IDLE);
}

void Jazz::SubUpdate(void)
{
	// 現在装備中の武器のポインタ
	WeaponBase* currentWeapon = nullptr;
	auto it = weaponSet_.find(currentWeaponID_);
	if (it != weaponSet_.end()) { currentWeapon = it->second; }

	// 現在の武器の攻撃アニメーションIDと比較する
	bool isAttacking = (currentWeapon != nullptr && (int)GetAnimePlayType() == currentWeapon->GetAttackAnimeID());

	if (isAttacking && !IsAnimeEnd())
	{
		Attack();
		return;
	}
	else if (isAttacking && IsAnimeEnd())
	{
		// 攻撃が終わったら、その武器の待機モーションに戻す
		if (currentWeapon != nullptr) {
			AnimePlay(currentWeapon->GetIdleAnimeID());
		}
	}

	if (!isAttacking) {
		// デフォルトは現在の武器の待機状態
		if (currentWeapon != nullptr) {
			AnimePlay(currentWeapon->GetIdleAnimeID());
		}
	}

	// 移動方向入力を取得
	Vector3 inputVec = InputVec();

	// 最終的に入力があれば加速度に加算する
	if (inputVec != 0.0f) {

		// 移動方向をカメラで回転させる
		inputVec.TransMatOwn(MGetRotY(CurrentCamera::Get().GetAngle().y));

		// 移動
		MoveAccel(inputVec);

		// 攻撃中でない場合のみ、移動アニメーションにする
		if (!isAttacking) { AnimePlay(CHARACTER_ANIME::WALK); }
	}

	// 攻撃ボタンの入力をチェック
	Attack();

	WeaponChange();

	trigger_.Update();
}

void Jazz::Attack(void)
{
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) {
		AnimePlay(CHARACTER_ANIME::SLASH_A1, false);
	}
}

void Jazz::WeaponChange(void)
{
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMainSwitch).down)
	{
		mainCurrentIndex_++;
		if (mainCurrentIndex_ >= trigger_.GetMainTrigger().size())
		{
			mainCurrentIndex_ = 0;
		}

		currentWeaponID_ = trigger_.GetMainTrigger()[mainCurrentIndex_];

		// もし変更先が「アステロイド」だった場合
		if (trigger_.GetMainTrigger()[mainCurrentIndex_] == TriggerUI::TriggerType::ASTEROID)
		{
			// 下位クラスを参照
			for (ActorBase* actor : GetChildActors()) {

				// 配列の中からアステロイドを探す
				if (auto asteroid = dynamic_cast<ShooterManager*>(actor)) {

					// アステロイドの準備
					asteroid->SetState(STATE::CUBE);

					// 終了
					break;
				}
			}
		}


		//std::cout << "武器を切り替えました: " << currentWeaponID_ << std::endl;
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