#include "Kogetsu.h"

#include "../../../pch.h"

#include "../../../Utility/Utility.h"

#include "../../Common/Collider/CapsuleCollider.h"

#include "../../Common/MeshSlicer/CutRequestQueue.h"

#include "../../Common/CuttableActor/CuttableActor.h"

#include "../../Common/Shader/RimLightShader.h"

Kogetsu::Kogetsu()
	:
	WeaponBase(Transform(), "Data/Parameter/Weapon/Kogetsu/"),
	isSheathed(false)
{
}

Kogetsu::Kogetsu(const Transform& ownerTrans)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Kogetsu/"),
	attackColl(new CapsuleCollider(COLLIDER_TAG::Kogetsu,
		BladeDirHand()* BLADE_START, BladeDirHand()* (BLADE_START + BLADE_LENGTH), BLADE_RADIUS)),
	isSheathed(false)
{
}

void Kogetsu::Load(void)
{
	trans.LoadModel("Weapon/Kogetsu");

	trans.SetLocalRotation(Quaternion::FromRotationZ(Deg2Rad(-90.0f)));

	SetJudgeFlg(false);

	// 動的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Movable);

	attackColl->SetJudgeFlg(false);

	AddCollider(attackColl);

	// 当たり判定による押し出しを有効にする
	SetContactResponseEnabled(false);
	
	// シェーダーを差し替える
	CreateShader(new RimLightShader(SHADER_VERTEX_TYPE::Frame1));
}

void Kogetsu::On(void)
{
	attackColl->SetJudgeFlg(true);
	slash_.Reset();
	cutId_ = CutRequestQueue::IssueCutId();   // 一振りごとに新しいID
}

void Kogetsu::Off(void)
{
	attackColl->SetJudgeFlg(false);
}

void Kogetsu::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	CuttableActor* target = dynamic_cast<CuttableActor*>(other.GetActor());
	if (target == nullptr) { return; }
	if (target->GetLastCutId() == cutId_) { return; }   // この一振りで切った相手（断片も）は除く

	CutPlane plane;
	if (!slash_.GetPlane(plane)) { return; }            // 突き・停止中は見送る
	CutRequestQueue::Push(target, plane, cutId_);
}

void Kogetsu::OnEquip(void)
{
	SetJudgeFlg(false);
	// 動的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Movable);
}

void Kogetsu::SubInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
}

namespace
{
	// 行列からスケールを取り除いた回転行列を返す(Rot:ロット=回転)
	MATRIX GetRotationWithoutScale(const MATRIX& mat)
	{
		const VECTOR scale = MGetSize(mat);
		return MMult(
			MGetRotElem(mat),
			MGetScale(VGet(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z))
		);
	}
}

// 更新処理: 右手に追従し、刀身の軌跡を更新する
void Kogetsu::SubUpdate(void)
{
	// ボーンが見つからないときは何もしない
	if (!FollowBone()) { return; }

	// 納刀中は斬撃を作らない
	if (!isSheathed) { UpdateSlash(); }
}

// 右手ボーンに合わせて、座標と回転を更新する(ボーンが無ければ false)
bool Kogetsu::FollowBone(void)
{
	// 追従するボーン(Frame:フレーム=モデルのボーン)を探す
	const char* boneName = isSheathed ? WAIST_BONE_NAME : HAND_BONE_NAME;
	const int frameIndex = MV1SearchFrame(ownerTrans.model, boneName);
	if (frameIndex == -1) { return false; }

	// ボーンのワールド行列(DxLibの関数がconstなし参照を受けるので const を付けない)
	MATRIX boneMat = MV1GetFrameLocalWorldMatrix(ownerTrans.model, frameIndex);

	// スケールを除いた回転行列
	const MATRIX boneRotationMat = GetRotationWithoutScale(boneMat);

	// 座標: ボーンの位置 + 回転だけで変換したずれ
	const Vector3 offset = isSheathed ? SHEATH_OFFSET : HAND_OFFSET;
	trans.pos =
		Vector3(MGetTranslateElem(boneMat)) +
		Vector3(VTransform(offset.ToVECTOR(), boneRotationMat));

	// 回転: ボーンの向き(納刀中は、さらに腰用の回転を加える)
	const Quaternion boneRotation = Quaternion::FromMatrix(boneRotationMat);
	trans.SetRotation(isSheathed ? boneRotation * SheathRotation() : boneRotation);

	return true;
}

// 刀身の根元と先端から、斬撃の軌跡を更新する
void Kogetsu::UpdateSlash(void)
{
	const Vector3 bladeDir = trans.VTrans(BladeDirHand());
	slash_.Update(
		trans.pos + bladeDir * BLADE_START,
		trans.pos + bladeDir * (BLADE_START + BLADE_LENGTH)
	);
}

void Kogetsu::SubDraw(void)
{
//#ifdef _DEBUG
//	const Vector3 o = trans.pos;
//	DrawLine3D(o.ToVECTOR(), (o + trans.VTrans(Vector3::Xonly(1.0f)) * 100.0f).ToVECTOR(), GetColor(255, 0, 0));     // X = 赤
//	DrawLine3D(o.ToVECTOR(), (o + trans.VTrans(Vector3::Yonly(1.0f)) * 100.0f).ToVECTOR(), GetColor(255, 255, 0));   // Y = 黄
//	DrawLine3D(o.ToVECTOR(), (o + trans.VTrans(Vector3::Zonly(1.0f)) * 100.0f).ToVECTOR(), GetColor(255, 255, 255)); // Z = 白
//#endif
}

void Kogetsu::SubRelease(void)
{
}
