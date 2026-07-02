#pragma once
#include "../CharacterBase.h"
#include <map>
#include <memory>
#include "../../Common/TriggerUI/TriggerUI.h"


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
	std::map<TriggerUI::TriggerType,std::unique_ptr<WeaponBase>> weaponSet_;

	//現在の装備を保存
	TriggerUI::TriggerType currentWeaponID_;

	int currentIndex_;


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
	//サブオブジェクト描画
	void SubObjectDraw(void)override;

	//攻撃
	void Attack(void);
	//武器の切り替え
	void WeaponChange(void);

private:

	//所持トリガーの表示
	TriggerUI trigger_;


	
};

