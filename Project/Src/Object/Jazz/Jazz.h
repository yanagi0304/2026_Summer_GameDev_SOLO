#pragma once

#include "../Common/CharacterBase/CharacterBase.h"

#include <map>
#include <memory>

#include "../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "../TriggerUI/TriggerUI.h"

class WeaponBase;

class Jazz : public CharacterBase
{
public:

	Jazz();
	~Jazz() override = default;

	void Load(void)override;

private:

	//武器の位置
	Transform weaponTrans_;
	Vector3 prevPos;

	//所持武器
	std::map<TriggerUI::TriggerType, WeaponBase*> weaponSet_;

	//現在の装備を保存
	TriggerUI::TriggerType currentWeaponID_;

	int mainCurrentIndex_;
	int subCurrentIndex_;

private:

	void SubInit(void)override;
	void SubUpdate(void)override;

	//攻撃
	void Attack(void);
	//武器の切り替え
	void WeaponChange(void);

	Vector3 InputVec(void) const;

private:

	//所持トリガーの表示
	TriggerUI trigger_;
};

