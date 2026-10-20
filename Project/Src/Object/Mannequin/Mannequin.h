#pragma once
#include "../Common/CuttableActor/CuttableActor.h"

// 弧月で切れるマネキン
// ・見た目と切断は CuttableActor が担当する（MV1モデルの形をCutMeshに取り込んで描く）
// ・床に立たせる・押し出す用のカプセルコライダーは今まで通り持つ
class Mannequin :
    public CuttableActor
{
public:
    Mannequin();
    ~Mannequin() = default;
    void Load(void) override;
    void SubInit(void) override;

    void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

private:


};
