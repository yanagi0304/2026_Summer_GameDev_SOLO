#include "Kogetsu.h"

#include "../../../pch.h"

#include "../../../Utility/Utility.h"

#include "../../Common/Collider/CapsuleCollider.h"

Kogetsu::Kogetsu()
	:
	WeaponBase(Transform(), "Data/Parameter/Weapon/Kogetsu/")
{
}

Kogetsu::Kogetsu(const Transform& ownerTrans)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Kogetsu/")
{
}

void Kogetsu::Load(void)
{
	trans.LoadModel("Weapon/Kogetsu");

	trans.SetLocalRotation(Quaternion::FromRotationY(Deg2Rad(180.0f)));

	SetJudgeFlg(true);
	SetDynamicFlg(true);
	SetPushFlg(true);

	//ColliderCreate(new CapsuleCollider(COLLIDER_TAG::KOGETSU, GetParameter("Collider", "ColliderStartPos")
	//,GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));
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
}

void Kogetsu::SubDraw(void)
{
	//auto frameIndex = MV1SearchFrame(ownerTrans_.model, "mixamorig:RightHand");
	//auto framePos = MV1GetFramePosition(ownerTrans_.model, frameIndex);
	//auto handMatrix = MV1GetFrameLocalWorldMatrix(ownerTrans_.model, frameIndex);
	//auto vec = VScale({ 0.0f,0.1f,0.0f }, 1.0 / 0.025);
	//auto swordStartPos = VTransform(vec, handMatrix);

	//VECTOR xAxis =
	//{
	//	handMatrix.m[0][0],
	//	handMatrix.m[0][1],
	//	handMatrix.m[0][2]
	//};

	//VECTOR yAxis =
	//{
	//	handMatrix.m[1][0],
	//	handMatrix.m[1][1],
	//	handMatrix.m[1][2]
	//};

	//VECTOR zAxis =
	//{
	//	handMatrix.m[2][0],
	//	handMatrix.m[2][1],
	//	handMatrix.m[2][2]
	//};

	//DrawLine3D(
	//	swordStartPos,
	//	VAdd(swordStartPos, VScale(xAxis, 20.0f)),
	//	GetColor(255, 0, 0));

	//DrawLine3D(
	//	swordStartPos,
	//	VAdd(swordStartPos, VScale(yAxis, 20.0f)),
	//	GetColor(0, 255, 0));

	//DrawLine3D(
	//	swordStartPos,
	//	VAdd(swordStartPos, VScale(zAxis, 20.0f)),
	//	GetColor(0, 0, 255));

}

void Kogetsu::SubRelease(void)
{
}
