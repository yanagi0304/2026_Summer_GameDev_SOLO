#pragma once

#include "../Common/CharacterBase/CharacterBase.h"

#include <map>
#include <memory>

#include "../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "../Weapon/TriggerTypeDefine.h"

class TriggerUI;

class WeaponBase;

class Jazz : public CharacterBase
{
public:

	Jazz();
	~Jazz() override = default;

	void Load(void)override;

	void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

private:

	//武器の位置
	Transform weaponTrans_;
	Vector3 prevPos;

	//所持武器
	std::map<TriggerType, WeaponBase*> weaponSet_;

	//現在の装備を保存
	TriggerType currentWeaponID_;

	int mainCurrentIndex_;
	int subCurrentIndex_;

private:

	void SubInit(void)override;
	void SubUpdate(void)override;
	
	//攻撃
	void Attack(void);
	//武器の切り替え（引数省略で次の番号のウェポンへ切り替え）
	void WeaponChange(TriggerType type = TriggerType::NONE);

	//移動
	Vector3 InputVec(void) const;

	void IsJump(void);

private:

	//所持トリガーの表示
	TriggerUI* trigger_;
};

