#pragma once

#include <functional>

#include "ShooterStateBase.h"

#include "../../../../Common/Vector3.h"

class AsteroidSwarm;
struct Transform;

// 27個に分割して並べている状態(ズームしてエイムできる)
class ShooterSplitState : public ShooterStateBase
{
public:

	// ownerTrans: 持ち主のTransform / swarm: 弾の群れ
	// getCenterPos: 立方体の中心位置を返す関数 / setAim: エイム(ズーム)のオンオフを行う関数
	ShooterSplitState(
		const Transform& ownerTrans,
		AsteroidSwarm& swarm,
		const std::function<Vector3(void)>& getCenterPos,
		const std::function<void(bool)>& setAim
	);

	~ShooterSplitState() override = default;

	// 状態遷移後1度行う初期化処理(27個を並べて出す)
	void Enter(void) override;

	// 更新処理(並べ直しとエイム入力)
	void Update(void) override;

	// 状態遷移前に1度行う終了処理(ズームを戻す)
	void Exit(void) override;

	// 次の状態(発射)
	SHOOTER_STATE NextState(void) const override { return SHOOTER_STATE::Fire; }

	// 待機アニメーションのID
	int GetIdleAnimeID(void) const override;

private:

	// 持ち主のTransform
	const Transform& ownerTrans;

	// 弾の群れ
	AsteroidSwarm& swarm;

	// 立方体の中心位置を返す関数
	const std::function<Vector3(void)> getCenterPos;

	// エイム(ズーム)のオンオフを行う関数
	const std::function<void(bool)> setAim;
};
