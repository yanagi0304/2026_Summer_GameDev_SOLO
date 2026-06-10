#pragma once
#include "../WeaponBase.h"
class Kogetsu :
    public WeaponBase
{
public:

    Kogetsu();
	~Kogetsu() override = default;

private:

    void WeaponLoad(void) override;
    void WeaponInit(void) override;
    void WeaponUpdate(void) override;
    void WeaponDraw(void) override;
    void WeaponAlphaDraw(void) override;
    void WeaponUiDraw(void) override;
	void WeaponRelease(void) override;

};

