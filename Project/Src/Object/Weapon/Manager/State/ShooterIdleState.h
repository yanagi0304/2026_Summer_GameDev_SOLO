#pragma once

#include "ShooterStateBase.h"

class AsteroidSwarm;

// 何も出していない状態
class ShooterIdleState : public ShooterStateBase
{
public:

	// swarm: 弾の群れ
	ShooterIdleState(AsteroidSwarm& swarm);

	~ShooterIdleState() override = default;

	// 状態遷移後1度行う初期化処理(弾をすべて消す)
	void Enter(void) override;

	// 次の状態(手元に1個出す状態)
	SHOOTER_STATE NextState(void) const override { return SHOOTER_STATE::Cube; }

	// 待機アニメーションのID
	int GetIdleAnimeID(void) const override;

private:

	// 弾の群れ
	AsteroidSwarm& swarm;
};
