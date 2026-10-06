#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Shield :
    public WeaponBase
{
public:
    Shield();
    Shield(const Transform& ownerTrans);
    ~Shield() override = default;

    void Load(void) override;

    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::IDLE_TO_SWORD_L; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

private:

    void SubInit(void) override;
    void SubUpdate(void) override;

private:

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");

    MATRIX weaponMatrix_;
};

