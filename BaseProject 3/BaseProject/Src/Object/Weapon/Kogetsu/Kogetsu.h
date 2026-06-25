#pragma once
#include "../WeaponBase.h"
class Kogetsu :
    public WeaponBase
{
public:


	Kogetsu();
    Kogetsu(const Transform& ownerTrans);
	~Kogetsu() override = default;

    void Draw(void)override;

    int GetIdleAnimeID(void) const { return AnimationController::CHARACTER_ANIME::SWORD_IDLE; }
    int GetAttackAnimeID(void) const { return AnimationController::CHARACTER_ANIME::SLASH_A1; }

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");


private:

    void WeaponLoad(void) override;
    void WeaponInit(void) override;
    void WeaponUpdate(void) override;
    void WeaponDraw(void) override;
    void WeaponAlphaDraw(void) override;
    void WeaponUiDraw(void) override;
	void WeaponRelease(void) override;

    MATRIX weaponMatrix_;

};

