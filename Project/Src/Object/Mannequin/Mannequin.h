#pragma once
#include "../Common/CuttableActor/CuttableActor.h"

#include "../../Common/Vector3.h"

class WeaponBase;

class Mannequin :
    public CuttableActor
{
public:
    Mannequin(const Vector3& pos, bool AttackFlg, const Vector3& targetPos);
    ~Mannequin() = default;
    void Load(void) override;
    void SubInit(void) override;
    void SubUpdate(void)override;

    void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

private:

    WeaponBase* weapon;
    const Vector3& targetPos;

    // 向く速さ(1フレームで目標の向きへ寄せる割合。大きいほど素早く向く)
    static constexpr float TURN_RATE = 0.1f;

    // モデルの正面の補正角度(ラジアン)
    // モデルがターゲットの反対を向く場合は 3.14159265f(π) にする
    static constexpr float MODEL_FORWARD_OFFSET = 0.0f;

    //ターゲットの方向を向く
	void TargetLookAt(void);
};
