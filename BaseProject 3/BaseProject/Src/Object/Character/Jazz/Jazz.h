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


public:

	const float SIZE_X = GetParameter("UI", "SIZE_X");
	const float SIZE_Y = GetParameter("UI", "SIZE_Y");

	static constexpr float OFFSET = 20.0f;

private:

	//武器の位置
	Transform weaponTrans_;
	Vector3 prevPos;

	//所持武器
	std::map<std::string,std::unique_ptr<WeaponBase>> weaponSet_;

	//現在の装備を保存
	std::string currentWeaponID_;

	//所持トリガー表示UI
	int triggerUI_;


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
	//武器の切り替え
	void WeaponChange(void);


	
};

