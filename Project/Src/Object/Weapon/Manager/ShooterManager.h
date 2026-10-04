#pragma once
#include "../WeaponBase.h"

#include <vector>
#include <memory>

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Asteroid;

enum class STATE
{
	CUBE,
	SPLIT,
	FIRE,
};

class ShooterManager
	: public WeaponBase
{
public:

	ShooterManager();
	ShooterManager(const Transform& ownerTrans);

	~ShooterManager() = default;

	void Load(void)override;

	int GetIdleAnimeID(void) const { return CHARACTER_ANIME::AsteroidIdle; }
	int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

	//ステートのセッター
	void SetState(const STATE state) { state_ = state; }

	static constexpr int div = 27;

	static constexpr int FIRE_INTERVAL = 3;

private:

	void SubUpdate(void) override;

	//ステート切り替え
	void ChangeState(void);

	//各状態の更新
	void CubeUpdate();
	void SpritUpdate();
	void FireUpdate();

private:
	int divideNum_;

	STATE state_;
	Asteroid* shot[div];

   int fireIndex_ = 0;
   int fireTimer_ = 0;
	
};

