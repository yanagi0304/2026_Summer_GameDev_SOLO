#pragma once
#include "../Common/ActorBase/ActorBase.h"
class Mannequin :
    public ActorBase
{
public:
    Mannequin();
    ~Mannequin() = default;
    void Load(void) override;
    void SubInit(void) override;
    void SubUpdate(void)override;
    void SubDraw(void) override;

    void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

private:

    bool sliced = false;
    std::vector<MeshSlicer::Tri> tris;
    std::vector<int> tex;
    MeshSlicer::Piece pieces[2];

};

