#pragma once

#include <functional>
#include <map>

#include "../WeaponBase.h"

#include "AsteroidSwarm.h"
#include "Crosshair.h"
#include "ShooterConfig.h"
#include "State/ShooterStateBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class SmokeBurst;

class ShooterManager
	: public WeaponBase
{
public:

	// コンストラクタ
	// ownerTrans: 持ち主のTransform / config: 武器の設定 / playAnime: 状態が変わったときに呼ぶアニメ再生関数
	ShooterManager(
		const Transform& ownerTrans,
		const ShooterConfig& config,
		const std::function<void(void)>& playAnime
	);

	~ShooterManager() override = default;

	// 読み込み
	void Load(void) override;

	// エイム中の発射方向を計算する
	std::optional<Vector3> CalcAimDir(void) const;

	// 現在の状態に合った待機アニメーションのIDを取得する
	int GetIdleAnimeID(void) const override;

	// 攻撃アニメーションのIDを取得する
	int GetAttackAnimeID(void) const override { return CHARACTER_ANIME::SLASH_A1; }

	// 武器が選ばれたとき
	void OnEquip(void) override;

	// 武器が外れたとき
	void OnUnequip(void) override;

private:

	void SubUpdate(void) override;
	void SubDraw(void) override;
	void SubRelease(void) override;

	// 描画タイプは常にUI
	ACTOR_DRAW_TYPE DefaultDrawType(void) const override { return ACTOR_DRAW_TYPE::Ui; }

	// 状態を切り替える
	void ChangeState(SHOOTER_STATE next);

	// 右手の位置を取得する
	Vector3 GetHandPos(void) const;

	// アステロイド群の中心位置を取得する
	Vector3 GetCenterPos(void) const;

	// エイム(ズーム)のオンオフを設定する
	void SetAiming(bool aim) const;

	// 煙の生成
	void EmitSmoke(const Vector3& pos, const Vector3& normal);

private:

	// 中心位置の、手からのずらし量
	static constexpr float CENTER_OFFSET_RIGHT = 80.0f;

	// 中心位置の、手からのずらし量
	static constexpr float CENTER_OFFSET_UP = 60.0f;

	// エイム中に狙う点の、カメラからの距離
	static constexpr float AIM_TARGET_DISTANCE = 3000.0f;

	//スモークの生成量
	static constexpr int SMOKE_BURST_NUM = 6;

	// 武器の設定
	ShooterConfig config_;

	// 状態が変わったときに呼ぶアニメ再生関数
	std::function<void(void)> playAnime;

	// 弾の群れ
	AsteroidSwarm swarm_;

	// 画面中央の照準
	Crosshair crosshair_;

	// 状態の一覧
	std::map<SHOOTER_STATE, ShooterStateBase*> states_;

	// 現在の状態
	SHOOTER_STATE currentState_;

	// 煙
	std::vector<SmokeBurst*> smokes;
	//煙の発生回数
	int nextSmoke;
};
