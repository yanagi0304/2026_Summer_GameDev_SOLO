#pragma once
#include "../CharacterBase.h"
#include <map>
#include <memory>

class WeaponBase;

class Jazz :
	public CharacterBase
{
public:
	Jazz();
	~Jazz() override = default;

private:

	//•Ší‚ÌˆÊ’u
	Transform weaponTrans_;
	Vector3 prevPos;

	//Š•Ší
	std::map<bool,std::unique_ptr<WeaponBase>> weaponSet_;

private:

	void CharacterLoad(void)override;
	void CharactorInit(void)override;
	void CharactorUpdate(void)override;
	void CharactorDraw(void)override;
	void CharactorAlphaDraw(void)override;
	void CharacterUiDraw(void)override;
	void CharactorRelease(void)override;
};

