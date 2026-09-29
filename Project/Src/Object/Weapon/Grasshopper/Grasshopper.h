#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Grasshopper :
    public WeaponBase
{

public:
    Grasshopper();
    Grasshopper(const Transform& ownerTrans);
    ~Grasshopper() override = default;



    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::IDLE; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }



private:

    void WeaponLoad(void) override;
    void WeaponInit(void) override;
    void WeaponUpdate(void) override;
    void WeaponDraw(void) override;
    void WeaponAlphaDraw(void) override;
    void WeaponUiDraw(void) override;
    void WeaponRelease(void) override;

private:


    MATRIX weaponMatrix_;

};

