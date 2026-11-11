#pragma once
#include "../WeaponBase.h"

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "../../Common/MeshSlicer/SlashTracker.h"

#include "../../../Utility/Utility.h"

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

    //納刀中かどうかの設定
    void SetSheathed(bool sheathed) { isSheathed = sheathed; }

private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
	void SubRelease(void) override;

    // 右手ボーンに合わせて、座標と回転を更新する(ボーンが無ければ false)
    bool FollowBone(void);

    // 刀身の根元と先端から、斬撃の軌跡(Slash:スラッシュ=斬る)を更新する
    void UpdateSlash(void);

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

    // 手に持つときの、手のボーンからのずらし量
    const Vector3 HAND_OFFSET = GetParameterToVector3("Init", "HandOffset");
    // 腰に付けるときの、腰のボーンからのずらし量
    const Vector3 SHEATH_OFFSET = GetParameterToVector3("Init", "SheathOffset");
    // 腰に付けるときの追加回転(Z軸まわり、度数)
    const Vector3 SHEATH_ROTATION_OFFSET = GetParameter("Init", "SheathRotationOffset");

    MATRIX weaponMatrix_;

	//納刀中かどうかのフラグ
    bool isSheathed;

    // 手に持つときのボーン名
    static constexpr const char* HAND_BONE_NAME = "mixamorig:RightHand";
    // 腰に付けるときのボーン名
    static constexpr const char* WAIST_BONE_NAME = "mixamorig:Hips";

    // 腰に付けるときの追加回転
    Quaternion SheathRotation(void) const { return Quaternion::FromRotationY(Deg2Rad(SHEATH_ROTATION_OFFSET.x)); }

};

