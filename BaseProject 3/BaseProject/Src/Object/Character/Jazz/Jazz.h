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

	//武器のコライダーを合わせた全コライダーを取得
	std::vector<ColliderBase*> GetCollider(void)const override;

private:

	//武器の位置
	Transform weaponTrans_;
	Vector3 prevPos;

	//所持武器
	std::map<bool,std::unique_ptr<WeaponBase>> weaponSet_;

private:

	void CharacterLoad(void)override;
	void CharactorInit(void)override;
	void CharactorUpdate(void)override;
	void CharactorDraw(void)override;
	void CharactorAlphaDraw(void)override;
	void CharacterUiDraw(void)override;
	void CharactorRelease(void)override;

	//サブオブジェクト更新
	void SubObjectUpdate(void)override;

	//攻撃
	void Attack(void);
	
};

