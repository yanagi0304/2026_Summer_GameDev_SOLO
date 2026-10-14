#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class SphereCollider;

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


private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
	void SubRelease(void) override;

private:

    static constexpr float COLL_RADIUS = 10.0f;

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");

    MATRIX weaponMatrix_;

    // 生成するコライダー
    SphereCollider* attackColl;

};

