#pragma once
#include "../WeaponBase.h"
class Asteroid :
    public WeaponBase
{
public:


    Asteroid();
    Asteroid(const Transform& ownerTrans);
    ~Asteroid() override = default;

private:

    void WeaponLoad(void) override;
    void WeaponInit(void) override;
    void WeaponUpdate(void) override;
    void WeaponDraw(void) override;
    void WeaponAlphaDraw(void) override;
    void WeaponUiDraw(void) override;
    void WeaponRelease(void) override;
};

