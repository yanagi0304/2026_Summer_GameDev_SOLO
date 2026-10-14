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


	// 状態定義
	enum class STATE
	{
		None = -1,

		Idle,
		Move,
		Jump,
		Kogetsu,

		Max
	};

#pragma region アニメーション関係定義

	// アニメーション再生速度テーブル
	float ANIME_SPEED_TABLE[(int)CHARACTER_ANIME::MAX] =
	{
		1.0f,	// Idle

		1.0f,	// Walk
		1.5f,	// Run

		1.0f,	// Damage
		1.0f,	// Down
		1.0f,	// Death

		1.0f,	// KickDown

		1.0f,	// ForwardThrow
		1.0f,	// Toss

	};

	// アニメーションループ再生フラグテーブル
	const bool ANIME_LOOP_TABLE[(int)CHARACTER_ANIME::MAX] =
	{
		true,	// Idle

		true,	// Walk
		true,	// Run

		false,	// Damage
		false,	// Down
		false,	// Death

		false,	// KickDown

		false,	// ForwardThrow
		false,	// Toss

	};

#pragma endregion

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

	//武器の位置
	Transform weaponTrans_;
	Vector3 prevPos;

	//所持武器
	std::map<TriggerType, WeaponBase*> weaponSet_;

	//現在の装備を保存
	TriggerType currentWeaponID_;

	int mainCurrentIndex_;
	int subCurrentIndex_;

};

