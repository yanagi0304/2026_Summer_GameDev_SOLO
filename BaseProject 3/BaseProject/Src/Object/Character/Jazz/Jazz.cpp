#include "Jazz.h"
#include <DxLib.h>

#include "../../Common/Collider/SphereCollider.h"
#include "../../Common/Collider/CapsuleCollider.h"

#include "../../../Manager/Input/KeyManager.h"
#include "../../../Manager/Camera/Camera.h"

#include "../../Weapon/WeaponBase.h"
#include "../../Weapon/Kogetsu/Kogetsu.h"




Jazz::Jazz()
	:
	CharacterBase(100, 10, 5, 5, "Data/Parameter/Character/Player/Wo-Chamolenium/"),
	weaponTrans_(prevPos)
{
	isOwnOperator = true;
}

std::vector<ColliderBase*> Jazz::GetCollider(void) const
{
	std::vector<ColliderBase*> ret = {};

	//自身のコライダーを一時的に保管
	for (ColliderBase*& c : ActorBase::GetCollider()) { ret.emplace_back(c); }

	//武器のコライダーを保管
	auto it = weaponSet_.find(true);
	if (it != weaponSet_.end() && it->second != nullptr)
	{
		for (ColliderBase*& c : it->second->GetCollider()) {ret.emplace_back(c);}
	}

	return ret;
}


void Jazz::CharacterLoad(void)
{
	trans.Load("Character/Jazz/Wochamole");
	weaponTrans_.Load("Weapon/Kogetsu");

	//武器の設定
	weaponSet_.emplace(true, std::make_unique<Kogetsu>(trans));


	SetJudge(true);
	SetDynamicFlg(true);
	SetGravityFlg(true);

	//モデルのコライダー生成
	ColliderCreate(new CapsuleCollider(COLLIDER_TAG::PLAYER, GetParameterToVector3("Collider", "ColliderStartPos")
		, GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));


	CreateAnimationController();
	AddInFbxAnimation(CHARACTER_ANIME::MAX, 0.5f);

	weaponTrans_.scale = (0.6f);

	for (auto& w : weaponSet_) {
		if (w.first) {
			w.second->Load();
		}
	}

	animeType_ = CHARACTER_ANIME::SWORD_IDLE;

}

void Jazz::CharactorInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = GetParameterToVector3("Init", "Scale");
	
	AnimePlay((int)animeType_);
}

void Jazz::CharactorUpdate(void)
{
	
	bool isAttacking = (animeType_ == CHARACTER_ANIME::SLASH_A1);
	if (isAttacking && !IsAnimeEnd())
	{
		Attack(); 
		
		return;
	}
	else if (isAttacking && IsAnimeEnd())
	{
		// 攻撃アニメーションが終わったら自然に待機モーションに戻す
		animeType_ = CHARACTER_ANIME::SWORD_IDLE;
	}

	// 攻撃中でなければ、デフォルトは待機状態
	if (!isAttacking) {
		animeType_ = CHARACTER_ANIME::SWORD_IDLE;
	}

	Vector3 angle = Camera::GetIns().GetAngle();
	float sinY = sinf(angle.y);
	float cosY = cosf(angle.y);

	Vector3 localAngle = Vector3();

	static constexpr float MOVE_POWER = 3.2f;
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
			animeType_ = CHARACTER_ANIME::WALK;
		}
	}

	AnimePlay((int)animeType_);

	// 攻撃ボタンの入力をチェック
	Attack();
}
void Jazz::CharactorDraw(void)
{
	//weaponTrans_.Draw();
	for(auto& w : weaponSet_) {
		if (w.first) {
			w.second->Draw();
		}
	}
}

void Jazz::CharactorAlphaDraw(void)
{
	for (auto& w : weaponSet_) {
		if (w.first) {
			w.second->AlphaDraw();
		}
	}
}

void Jazz::CharacterUiDraw(void)
{
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
	for (auto& w : weaponSet_) {
		if (w.first) {
			w.second->Update();
		}
	}
}

void Jazz::Attack(void)
{
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MAIN).down) {
		animeType_ = CHARACTER_ANIME::SLASH_A1;
		AnimePlay((int)animeType_, false);
	}
}



