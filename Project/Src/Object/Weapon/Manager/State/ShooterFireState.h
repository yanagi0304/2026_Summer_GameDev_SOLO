#pragma once

#include "ShooterStateBase.h"

#include "../../../../Common/Vector3.h"
#include <optional>

class AsteroidSwarm;
struct Transform;

// 発射中の状態
class ShooterFireState : public ShooterStateBase
{
public:

	// ownerTrans: 持ち主のTransform/ swarm: 弾の群れ
	// getAimDir: エイム中の発射方向を返す関数
	ShooterFireState(const Transform& ownerTrans, AsteroidSwarm& swarm,
		const std::function<std::optional<Vector3>(void)>& getAimDir);

	~ShooterFireState() override = default;

	// 状態遷移後1度行う初期化処理(発射方向を決めて、全部撃つ)
	void Enter(void) override;

	// 状態遷移前に1度行う終了処理(飛んでいる弾を手元に戻す)
	void Exit(void) override;

	// 次の状態(手元に1個出す状態に戻る)
	SHOOTER_STATE NextState(void) const override { return SHOOTER_STATE::Cube; }

	// 待機アニメーションのID
	int GetIdleAnimeID(void) const override;

private:

	// 発射方向を決める
	void UpdateFireDir(void);

private:

	// 持ち主のTransform
	const Transform& ownerTrans;

	// 弾の群れ
	AsteroidSwarm& swarm;

	// 発射方向
	Vector3 fireDir;

	// エイム中の発射方向を返す関数
	const std::function<std::optional<Vector3>(void)> getAimDir;

};
