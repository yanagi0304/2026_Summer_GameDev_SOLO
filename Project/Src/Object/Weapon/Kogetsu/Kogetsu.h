#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Kogetsu :
    public WeaponBase
{
public:


	Kogetsu();
    Kogetsu(const Transform& ownerTrans);
	~Kogetsu() override = default;

    void Load(void) override;

    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::SWORD_IDLE_R; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");


private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
	void SubRelease(void) override;

    MATRIX weaponMatrix_;

};

