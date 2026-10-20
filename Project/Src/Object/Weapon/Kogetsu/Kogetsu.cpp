#include "Kogetsu.h"

#include "../../../pch.h"

#include "../../../Utility/Utility.h"

#include "../../Common/Collider/CapsuleCollider.h"

#include "../../Common/MeshSlicer/CutRequestQueue.h"

#include "../../Common/CuttableActor/CuttableActor.h"

Kogetsu::Kogetsu()
	:
	WeaponBase(Transform(), "Data/Parameter/Weapon/Kogetsu/")
{
}

Kogetsu::Kogetsu(const Transform& ownerTrans)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Kogetsu/"),
	attackColl(new CapsuleCollider(COLLIDER_TAG::Kogetsu,
		BladeDirHand()* BLADE_START, BladeDirHand()* (BLADE_START + BLADE_LENGTH), BLADE_RADIUS))
{
}

void Kogetsu::Load(void)
{
	trans.LoadModel("Weapon/Kogetsu");

	trans.SetLocalRotation(Quaternion::FromRotationZ(Deg2Rad(-90.0f)));

	SetJudgeFlg(true);
	SetDynamicFlg(true);
	SetPushFlg(false);

	attackColl->SetJudgeFlg(false);

	AddCollider(attackColl);
	
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

void Kogetsu::SubInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
}

void Kogetsu::SubUpdate(void)
{
	const int frameIndex =
		MV1SearchFrame(
			ownerTrans_.model,
			"mixamorig:RightHand"
		);

	if (frameIndex == -1)
	{
		return;
	}

	// 右手ボーンのワールド行列
	MATRIX handMat =
		MV1GetFrameLocalWorldMatrix(
			ownerTrans_.model,
			frameIndex
		);


	// ========================================
	// 回転行列を取得
	// ========================================

	const VECTOR handScale =
		MGetSize(handMat);

	MATRIX handRotationMat =
		MGetRotElem(handMat);

	// スケールを除去
	handRotationMat =
		MMult(
			handRotationMat,
			MGetScale(
				VGet(
					1.0f / handScale.x,
					1.0f / handScale.y,
					1.0f / handScale.z
				)
			)
		);


	// ========================================
	// 座標
	// ========================================

	const Vector3 handPos =
		Vector3(
			MGetTranslateElem(handMat)
		);

	// OFFSETは「回転だけ」で変換する
	const Vector3 offset =
		Vector3(
			VTransform(
				OFFSET.ToVECTOR(),
				handRotationMat
			)
		);

	trans.pos =
		handPos +
		offset;


	// ========================================
	// 回転
	// ========================================

	trans.SetRotation(
		Quaternion::FromMatrix(
			handRotationMat
		)
	);

	// SubUpdate の最後に追加
	const Vector3 bladeDir = trans.VTrans(BladeDirHand());
	slash_.Update(trans.pos + bladeDir * BLADE_START, trans.pos + bladeDir * (BLADE_START + BLADE_LENGTH));

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
