#pragma once

#include "../Common/CharacterBase/CharacterBase.h"
#include "../Common/Effect/PolyDecompose/PolyDecompose.h"

#include <map>
#include <memory>

#include "../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "../Weapon/TriggerTypeDefine.h"
#include "../Weapon/LockOn/LockOnSystem.h"

class TriggerUI;
class WeaponBase;


class Jazz : public CharacterBase
{
public:

	Jazz();
	~Jazz() override = default;

	void Load(void)override;

	// 描画(生成演出中は、ポリゴン分解で描く)
	void Draw(void)override;

	// 生成演出の最中か
	bool IsSpawning(void)const { return spawnEffect != nullptr; }

	// 生成演出の設定(長さ・破片の動き)を変える。シーンに追加する前に呼ぶ
	void SetSpawnParams(const PolyDecompose::Params& params) { spawnParams = params; }

	void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

	ACTOR_DRAW_TYPE DefaultDrawType(void)const override { return ACTOR_DRAW_TYPE::Alpha; }

	// ロックオンできる対象の一覧を作る関数を設定する
	void SetLockCandidateSource(const LockOnSystem::CollectFunc& collect) { lockOn.SetCandidateSource(collect); }


private:


	// 状態定義
	enum class STATE
	{
		None = -1,

		Idle,
		Move,
		Jump,
		Kogetsu,
		GrasshopperJump,

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

		0.7,	// SlashA1
		0.7,	// SlashA2
		0.6,	// SlashA3

		0.5,	// AsteroidIdle.R
		0.5,	// AsteroidCreate.R
		1.5f,	// AsteroidIdleShot.R

		1.5,	// JumpStart
		0.5,	// Jumping
		0.5,	// JumpEnd

		0.5,	// Run

		0.5,	// Sheathed
		0.5,	// Draw

		0.8f,	// NinjaRun


	};

	// アニメーションループ再生フラグテーブル
	const bool ANIME_LOOP_TABLE[(int)CHARACTER_ANIME::MAX] =
	{
		true,	// Idle

		true,	// Walk

		true,	// SwordIdle.R
		true,	// ShildIdle.L

		false,	// SlashA1
		false,	// SlashA2
		false,	// SlashA3

		true,	// AsteroidIdle.R
		true,	// AsteroidCreate.R 
		true,	// AsteroidIdleShot.R

		false,	// JumpStart
		true,	// Jumping
		false,	// JumpEnd

		true,	// Run

		false,	// Sheathed
		false,	// Draw

		true,	// NinjaRun

	};

#pragma endregion

private:

	// 武器スロット(所持武器・現在の武器・UI用の番号をまとめたもの)
	struct WeaponSlot
	{
		std::map<TriggerType, WeaponBase*> set;
		TriggerType currentID = TriggerType::NONE;
		int currentIndex = 0;
	};

	void SubInit(void)override;
	void SubUpdate(void)override;
	void SubRelease(void)override;

	// グラスホッパーで打ち上げられたときの状態遷移
	void OnGrasshopperLaunch(bool isStraightUp);

	//武器切り替えできるか取得
	bool CanWeaponChange(void) const;

	// 状態に合わせて、弧月を手に持つか腰に付けるかを切り替える
	void UpdateKogetsuSheath(void);
	
	//武器の切り替え（引数省略で次の番号のウェポンへ切り替え）
	void WeaponChange(TriggerType type = TriggerType::NONE);

	// 指定スロットの武器を切り替える(NONEなら次の武器へ)
	void SwitchWeapon(WeaponSlot& slot,  const std::vector<TriggerType>& triggers,TriggerType type = TriggerType::NONE );

	Vector3 InputVec(void) const;

	void SubDraw(void)override;

	//所持トリガーの確認
	bool IsTrigger(TriggerType trigger);

	//エイム中のプレイヤー回転
	void AimRotate(void);

private:

	//所持トリガーの表示
	TriggerUI* trigger;

	//武器の位置
	Transform weaponTrans;
	Vector3 prevPos;

	// メイン/サブそれぞれの武器スロット
	WeaponSlot mainSlot;
	WeaponSlot subSlot;

	//エイム中か
	bool isAim;

	// ハウンドのロックオン
	LockOnSystem lockOn;

	// 生成演出
	PolyDecompose* spawnEffect;

	// 生成演出の経過フレーム
	float spawnFrame;

	// 生成演出の設定(初期値はPolyDecompose::Paramsの固定値)
	PolyDecompose::Params spawnParams;

#pragma region 外部パラメータの読み込み

	const float CAPSULE_RADIUS = GetParameter("Collider", "Radius");	// カプセルコライダーの半径
	const Vector3 CAPSULE_START = GetParameterToVector3("Collider", "ColliderStartPos");	// カプセルコライダーの高さの半分
	const Vector3 CAPSULE_END = GetParameterToVector3("Collider", "ColliderEndPos");		// カプセルコライダーの高さの半分

#pragma endregion

};

