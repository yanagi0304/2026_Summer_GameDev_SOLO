#include "Jazz.h"
#include <DxLib.h>

#include "../../../Application/Application.h"

#include "../../Common/Collider/SphereCollider.h"
#include "../../Common/Collider/CapsuleCollider.h"

#include "../../../Manager/Input/KeyManager.h"
#include "../../../Manager/Camera/Camera.h"

#include "../../Weapon/WeaponBase.h"
#include "../../Weapon/Kogetsu/Kogetsu.h"
#include "../../Weapon/Asteroid/Asteroid.h"
#include "../../Weapon/Manager/ShooterManager.h"
#include <iostream>




Jazz::Jazz()
	:
	CharacterBase(100, 10, 5, 5, "Data/Parameter/Character/Player/Wo-Chamolenium/"),
	weaponTrans_(prevPos),
	currentIndex_(0)
{
	isOwnOperator = true;
}

std::vector<ColliderBase*> Jazz::GetCollider(void) const
{
	std::vector<ColliderBase*> ret = {};

	// 自身のコライダーを一時的に保管
	for (ColliderBase*& c : ActorBase::GetCollider()) { ret.emplace_back(c); }

	//装備している武器を送る
	auto it = weaponSet_.find(currentWeaponID_);
	if (it != weaponSet_.end() && it->second != nullptr)
	{
		for (ColliderBase*& c : it->second->GetCollider()) { ret.emplace_back(c); }
	}

	return ret;
}


void Jazz::CharacterLoad(void)
{
	trans.Load("Character/Jazz/Wochamole");
	

	//武器の設定
	weaponSet_.emplace(TriggerUI::TriggerType::KOGETSU, std::make_unique<Kogetsu>(trans));
	weaponSet_.emplace(TriggerUI::TriggerType::ASTEROID, std::make_unique<ShooterManager>(trans));


	SetJudge(true);
	SetDynamicFlg(true);
	SetGravityFlg(true);

	//モデルのコライダー生成
	ColliderCreate(new CapsuleCollider(COLLIDER_TAG::PLAYER, GetParameterToVector3("Collider", "ColliderStartPos")
		, GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));


	CreateAnimationController();
	AddInFbxAnimation(AnimationController::CHARACTER_ANIME::MAX, 0.5f);

	for (auto& w : weaponSet_) {
		w.second->Load();
		trigger_.AddMain(w.first);
	}

	animeType_ = AnimationController::CHARACTER_ANIME::SWORD_IDLE;

	currentWeaponID_ = TriggerUI::TriggerType::KOGETSU;

	trigger_.Load();



}

void Jazz::CharactorInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = GetParameterToVector3("Init", "Scale");
	AnimePlay((int)animeType_);

}

void Jazz::CharactorUpdate(void)
{
	
	// 現在装備中の武器のポインタ
	WeaponBase* currentWeapon = nullptr;
	auto it = weaponSet_.find(currentWeaponID_);
	if (it != weaponSet_.end()) { currentWeapon = it->second.get(); }


	// 現在の武器の攻撃アニメーションIDと比較する
	bool isAttacking = (currentWeapon != nullptr && (int)animeType_ == currentWeapon->GetAttackAnimeID());

	if (isAttacking && !IsAnimeEnd())
	{
		Attack();
		return;
	}
	else if (isAttacking && IsAnimeEnd())
	{
		// 攻撃が終わったら、その武器の待機モーションに戻す
		if (currentWeapon != nullptr) {
			animeType_ = (AnimationController::CHARACTER_ANIME)currentWeapon->GetIdleAnimeID();
		}
	}

	if (!isAttacking) {
		// デフォルトは現在の武器の待機状態
		if (currentWeapon != nullptr) {
			animeType_ = (AnimationController::CHARACTER_ANIME)currentWeapon->GetIdleAnimeID();
		}
	}

	Vector3 angle = Camera::GetIns().GetAngle();
	float sinY = sinf(angle.y);
	float cosY = cosf(angle.y);

	Vector3 localAngle = Vector3();

	static constexpr float MOVE_POWER = 4.0f;
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_FRONT).now) { localAngle.z += MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_BACK).now) { localAngle.z -= MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_LEFT).now) { localAngle.x -= MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_RIGHT).now) { localAngle.x += MOVE_POWER; }

	if (localAngle != 0.0f)
	{
		trans.angle.y = atan2f(-localAngle.x, -localAngle.z) + angle.y;
		accelSum.x += localAngle.x * cosY + localAngle.z * sinY;
		accelSum.z += localAngle.z * cosY - localAngle.x * sinY;

		// 攻撃中でない場合のみ、移動アニメーションにする
		if (!isAttacking) {
			animeType_ = AnimationController::CHARACTER_ANIME::WALK;
		}
	}

	AnimePlay((int)animeType_);

	// 攻撃ボタンの入力をチェック
	Attack();

	WeaponChange();

	trigger_.Update();
}
void Jazz::CharactorDraw(void)
{
	auto it = weaponSet_.find(currentWeaponID_);
	if (it != weaponSet_.end() && it->second != nullptr)
	{
		it->second->Draw();
	}
}

void Jazz::CharactorAlphaDraw(void)
{
	auto it = weaponSet_.find(currentWeaponID_);
	if (it != weaponSet_.end() && it->second != nullptr)
	{
		it->second->AlphaDraw();
	}
}

void Jazz::CharacterUiDraw(void)
{
	trigger_.Draw(currentIndex_);
}

void Jazz::CharactorRelease(void)
{
	for (auto& w : weaponSet_) {
		if (!w.second) { continue; }
		delete w.second.release();
		w.second = nullptr;
	}
	weaponSet_.clear();
}

void Jazz::SubObjectUpdate(void)
{
	auto it = weaponSet_.find(currentWeaponID_);
	if (it != weaponSet_.end() && it->second != nullptr)
	{
		it->second->Update();
	}
}

void Jazz::Attack(void)
{
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MAIN).down) {
		animeType_ = AnimationController::CHARACTER_ANIME::SLASH_A1;
		AnimePlay((int)animeType_, false);
	}
}

void Jazz::WeaponChange(void)
{
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MAIN_SWITCH).down)
	{
		currentIndex_++;
		if (currentIndex_ >= trigger_.GetMainTrigger().size())
		{
			currentIndex_ = 0;
		}

		currentWeaponID_ = trigger_.GetMainTrigger()[currentIndex_];

		//std::cout << "武器を切り替えました: " << currentWeaponID_ << std::endl;
	}
}



