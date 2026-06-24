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
	//各状態の描画
	void CubeDraw();
	void SpritDraw();

private:

	STATE state_;
   std::unique_ptr<WeaponBase> shot[8];
	
};

