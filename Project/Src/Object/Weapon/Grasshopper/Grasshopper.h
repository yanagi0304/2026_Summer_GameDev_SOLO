#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Grasshopper :
    public WeaponBase
{

public:
    Grasshopper(const Transform& ownerTrans,bool& isGrashpper);
    ~Grasshopper() override = default;

    void Load(void) override;

    int GetIdleAnimeID(void) const { return CHARACTER_ANIME::IDLE; }
    int GetAttackAnimeID(void) const { return CHARACTER_ANIME::SLASH_A1; }

    void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

    void OnEquip(void)override;

private:

    //武器生成
    void Create(void);

    //入力方向取得
    Vector3 InputVec(void) const;

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
    void SubRelease(void) override;

private:

    MATRIX weaponMatrix_;

    //生成時の傾き
    MATRIX rotationMat;

    float tiltPitch_;   // 現在のピッチ角(ラジアン)
    float tiltRoll_;   // 現在のロール角(ラジアン)

#pragma region 受け取り参照
    bool& isGrashpper;
#pragma endregion

};

