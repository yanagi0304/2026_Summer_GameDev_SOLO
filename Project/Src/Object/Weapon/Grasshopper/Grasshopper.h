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

    void Load(void) override;

    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::IDLE; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
    void SubRelease(void) override;

private:

    MATRIX weaponMatrix_;
};

