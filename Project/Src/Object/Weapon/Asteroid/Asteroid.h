#pragma once

#include <deque>

#include "../WeaponBase.h"

#include "../Manager/ShooterConfig.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Asteroid :
	public WeaponBase
{
public:

	// 弾の状態
	enum class STATE
	{
		// 手元で待機している
		Hold,

		// 発射されて飛んでいる
		Flying,

		// 命中して消えた
		Dead,
	};

	// コンストラクタ
	// ownerTrans: 持ち主(Jazz)のTransform / config: 弾の設定
	Asteroid(const Transform& ownerTrans, const ShooterConfig& config);

	~Asteroid() override = default;

	// 読み込み
	void Load(void) override;

	// 待機アニメーションのIDを取得する
	int GetIdleAnimeID(void) const override { return CHARACTER_ANIME::SWORD_IDLE_R; }

	// 攻撃アニメーションのIDを取得する
	int GetAttackAnimeID(void) const override { return CHARACTER_ANIME::SLASH_A1; }

	// 元の大きさを取得する
	float GetBaseScale(void) const { return baseScale_; }

	// 手元に出す
	void Appear(void);

	// 消す
	void Disappear(void);

	// 座標と大きさを設定する
	void Place(const Vector3& pos, const Vector3& scale);

	// 発射する
	void Fire(const Vector3& dir);

	// 手元の状態に戻す
	void Recall(void);

	// 現在の状態を取得する
	STATE GetState(void) const { return state_; }

	// 当たり判定の通知
	void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result) override;

	//デフォルト描画タイプの設定
	ACTOR_DRAW_TYPE DefaultDrawType(void)const override { return ACTOR_DRAW_TYPE::Alpha; }

private:

	void SubInit(void) override;
	void SubUpdate(void) override;
	void SubDraw(void) override;
	void SubRelease(void) override;

	// 動きをリセットする
	void ResetMotion(void);

	// 命中して消える
	void Kill(void);

	// 目標へ向きを寄せる
	void Steer(const Vector3& targetPos);

private:

	// 軌跡として覚えておく最大数
	static constexpr int TRAIL_MAX = 10;

	// 飛行中の加速度
	static constexpr float FIRE_ACCEL = 1.0f;

	// 追尾の旋回の強さ
	static constexpr float HOMING_TURN_RATE = 0.08f;

	// 現在の状態
	STATE state_;

	// 進行方向
	Vector3 moveDir_;

	// 軌跡
	std::deque<Vector3> trail_;

	// 元の大きさ
	float baseScale_;

	// 弾の設定
	ShooterConfig config_;

};
