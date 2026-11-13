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

    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::SWORD_IDLE_TO_SHILD_L; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

    //デフォルト描画タイプの設定
    ACTOR_DRAW_TYPE DefaultDrawType(void)const override { return ACTOR_DRAW_TYPE::Alpha; }

private:

    void SubInit(void) override;
    void SubUpdate(void) override;

private:

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");

    MATRIX weaponMatrix_;
};

