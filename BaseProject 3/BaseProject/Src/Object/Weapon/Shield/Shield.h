#pragma once
#include "../WeaponBase.h"
class Shield :
    public WeaponBase
{
public:
    Shield();
    Shield(const Transform& ownerTrans);
    ~Shield() override = default;

   

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

private:

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");


    MATRIX weaponMatrix_;
};

