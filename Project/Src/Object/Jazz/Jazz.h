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

	Jazz(std::function<const Vector3&(void)> target);
	~Jazz() override = default;

	void Load(void)override;

	void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

	// ターゲットとしての陣営
	TARGET_TEAM GetTargetTeam(void) const override { return TARGET_TEAM::Ally; }

	//グラスホッパーの使用状況取得
	bool GetIsGrashopper(void) const { return isGrashpper; }

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
		0.5,	// Idle

		0.5,	// Walk

		0.5,	// SwordIdle.R
		0.5,	// ShildIdle.L

		0.5,	// Jump

		0.7,	// SlashA1
		0.7,	// SlashA2
		0.6,	// SlashA3

		0.5,	// AsteroidIdle.R
		0.5,	// AsteroidCreate.R
		1.5f,	// AsteroidIdleShot.R

	};

	// アニメーションループ再生フラグテーブル
	const bool ANIME_LOOP_TABLE[(int)CHARACTER_ANIME::MAX] =
	{
		true,	// Idle

		true,	// Walk

		true,	// SwordIdle.R
		true,	// ShildIdle.L

		false,	// Jump

		false,	// SlashA1
		false,	// SlashA2
		false,	// SlashA3

		true,	// AsteroidIdle.R
		true,	// AsteroidCreate.R 
		true,	// AsteroidIdleShot.R

	};

#pragma endregion

private:

	void SubInit(void)override;
	void SubUpdate(void)override;

	//武器切り替えできるか取得
	bool CanWeaponChange(void) const;
	
	//武器の切り替え（引数省略で次の番号のウェポンへ切り替え）
	void WeaponChange(TriggerType type = TriggerType::NONE);

	Vector3 InputVec(void) const;

	void SubDraw(void)override;

	void IsJump(void);

	//所持トリガーの確認
	bool IsTrigger(TriggerType trigger);

private:

	//所持トリガーの表示
	TriggerUI* trigger;

	//武器の位置
	Transform weaponTrans;
	Vector3 prevPos;

	//所持武器
	std::map<TriggerType, WeaponBase*> mainWeaponSet;
	std::map<TriggerType, WeaponBase*> subWeaponSet;

	//現在の装備を保存
	TriggerType currentWeaponID;

	int mainCurrentIndex;
	int subCurrentIndex;

	//現在グラスホッパーの使用をしているか
	bool isGrashpper;

#pragma region 受け取り参照

	std::function<const Vector3& (void)> target;

#pragma endregion

};

