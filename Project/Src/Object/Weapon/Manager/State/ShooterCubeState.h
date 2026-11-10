#pragma once

#include <functional>

#include "ShooterStateBase.h"

#include "../../../../Common/Vector3.h"

class AsteroidSwarm;

// 手元にアステロイドが1個ある状態
class ShooterCubeState : public ShooterStateBase
{
public:

	// swarm: 弾の群れ / getCenterPos: 弾を出す位置を返す関数
	ShooterCubeState(AsteroidSwarm& swarm, const std::function<Vector3(void)>& getCenterPos);

	~ShooterCubeState() override = default;

	// 状態遷移後1度行う初期化処理(1個だけ出す)
	void Enter(void) override;

	// 更新処理(手元の位置へ追従させる)
	void Update(void) override;

	// 次の状態(27個に分割する状態)
	SHOOTER_STATE NextState(void) const override { return SHOOTER_STATE::Split; }

	// 待機アニメーションのID
	int GetIdleAnimeID(void) const override;

private:

	// 弾の群れ
	AsteroidSwarm& swarm;

	// 弾を出す位置を返す関数
	const std::function<Vector3(void)> getCenterPos;
};
