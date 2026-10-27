#pragma once
#include "../ActorBase/ActorBase.h"

class SweepLineCollider :
    public ActorBase
{
public:

    SweepLineCollider(const Vector3& pos);
    ~SweepLineCollider()override = default;

    void Load(void)override;
    void SubUpdate(void)override;
    void SubDraw(void)override;

private:

    //ü•ªÀ•W
    Vector3 currPos;
    Vector3 prevPos;

};

