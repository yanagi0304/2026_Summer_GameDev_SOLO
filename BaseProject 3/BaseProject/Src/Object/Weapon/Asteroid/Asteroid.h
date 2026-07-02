#pragma once
#include "../WeaponBase.h"


class Asteroid :
    public WeaponBase
{
public:

    const float SIZE = GetParameter("Init", "scale");

    Asteroid();
    Asteroid(const Transform& ownerTrans);
    ~Asteroid() override = default;

    int GetIdleAnimeID(void) const { return AnimationController::CHARACTER_ANIME::AsteroidIdle; }
    int GetAttackAnimeID(void) const { return AnimationController::CHARACTER_ANIME::SLASH_A1; }
    void SetTrans(const Vector3 pos, const Vector3 scale) { trans.pos = pos; trans.scale = scale; }
    void SetDir(const Vector3 dir) { direction_ = dir; }
    Vector3 GetDir(void) const { return direction_; }
    float GetSize(void) const { return SIZE; }

    void SetFire(bool fire) { isFire_ = fire; }
    bool IsFire(void) const { return isFire_; }

private:

    void WeaponLoad(void) override;
    void WeaponInit(void) override;
    void WeaponUpdate(void) override;
    void WeaponDraw(void) override;
    void WeaponAlphaDraw(void) override;
    void WeaponUiDraw(void) override;
    void WeaponRelease(void) override;

private:
    Vector3 direction_;
    bool isFire_ = false;
};

