#pragma once
#include <deque>

#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Asteroid :
    public WeaponBase
{
public:

    const float SIZE = GetParameter("Init", "scale");

    Asteroid();
    ~Asteroid() override = default;

    void Load(void) override;

    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::SWORD_IDLE_R; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

    void SetTrans(const Vector3 pos, const Vector3 scale) { trans.pos = pos; trans.scale = scale; }

    void SetDir(const Vector3 dir) { direction_ = dir; }

    Vector3 GetDir(void) const { return direction_; }

    float GetSize(void) const { return SIZE; }

    //î≠éÀèÛãµ
    void SetFire(bool fire) { isFire = fire; }
    bool IsFire(void) const { return isFire; }

    //ãOê’Çèâä˙âª
    void ClearTrail(void) { trail.clear(); }

    void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

    //ëIÇŒÇÍÇΩÇ∆Ç´
    void OnEquip(void)override;


private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
    void SubRelease(void) override;

private:

    Vector3 direction_;
    bool isFire = false;

    //ãOê’ï€ä«
    static constexpr int TRAIL_MAX = 10;

    std::deque<Vector3> trail;
};

