#pragma once
#include "../WeaponBase.h"
#include <vector>
#include <memory>

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
	int GetIdleAnimeID(void) const { return AnimationController::CHARACTER_ANIME::AsteroidIdle; }
	int GetAttackAnimeID(void) const { return AnimationController::CHARACTER_ANIME::SLASH_A1; }

	//ステートのセッター
	void SetState(const STATE state) { state_ = state; }

	static constexpr int div = 27;

	static constexpr int FIRE_INTERVAL = 3;

private:

	void WeaponLoad(void) override;
	void WeaponInit(void) override;
	void WeaponUpdate(void) override;
	void WeaponDraw(void) override;
	void WeaponAlphaDraw(void) override;
	void WeaponUiDraw(void) override;
	void WeaponRelease(void) override;

	//ステート切り替え
	void ChangeState(void);

	//各状態の更新
	void CubeUpdate();
	void SpritUpdate();
	void FireUpdate();
	//各状態の描画
	void CubeDraw();
	void SpritDraw();
	void FireDraw();

private:
	int divideNum_;

	STATE state_;
   std::unique_ptr<WeaponBase> shot[27];

   int fireIndex_ = 0;
   int fireTimer_ = 0;
	
};

