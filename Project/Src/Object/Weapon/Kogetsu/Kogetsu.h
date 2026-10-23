#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "../../Common/MeshSlicer/SlashTracker.h"

class CapsuleCollider;

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

    void On(void) override;
    void Off(void) override;

    void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

    void OnEquip(void)override;

private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
	void SubRelease(void) override;

private:

    static constexpr float COLL_RADIUS = 10.0f;

    // 刃の向き（手のボーン基準）。逆向きなら Yonly(-1.0f) や Xonly / Zonly に変える
    static Vector3 BladeDirHand(void) { return Vector3::Xonly(1.0f); }
    static constexpr float BLADE_START = 0.0f;     // 柄の原点から刃の根本まで
    static constexpr float BLADE_LENGTH = 100.0f;   // 刃の長さ
    static constexpr float BLADE_RADIUS = 8.0f;     // 当たり判定の太さ（COLL_RADIUS は削除）
    CapsuleCollider* attackColl;                    // SphereCollider* から変更
    SlashTracker slash_;
    int cutId_ = -1;

    const Vector3 OFFSET = GetParameterToVector3("Init", "modelOffset");

    MATRIX weaponMatrix_;

};

