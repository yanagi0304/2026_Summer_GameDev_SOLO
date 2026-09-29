#include "Kogetsu.h"
#include "../../Common/Collider/CapsuleCollider.h"
#include "../../../Utility/Utility.h"
#include <DxLib.h>

Kogetsu::Kogetsu()
	:
	WeaponBase("Data/Parameter/Weapon/Kogetsu/")
{
}

Kogetsu::Kogetsu(const Transform& ownerTrans)
	:
	WeaponBase("Data/Parameter/Weapon/Kogetsu/")
{
	ownerTrans_ = ownerTrans;
}

void Kogetsu::Draw(void)
{
	WeaponDraw();
	
	MV1DrawModel(trans.model);
}

void Kogetsu::WeaponLoad(void)
{
	trans.Load("Weapon/Kogetsu");
	WeaponInit();

	SetJudge(true);
	SetDynamicFlg(true);
	SetPushFlg(true);

	//ColliderCreate(new CapsuleCollider(COLLIDER_TAG::KOGETSU, GetParameter("Collider", "ColliderStartPos")
	//,GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));
}

void Kogetsu::WeaponInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));

}

void Kogetsu::WeaponUpdate(void)
{
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHand");
	auto framePos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);
	auto handMatrix = MV1GetFrameLocalWorldMatrix(ownerTrans_->get().model, frameIndex);

	//trans.pos = framePos;
	//trans.angle = MatrixToEulerXYZ(handMatrix);

	MATRIX scaleMat = MGetScale(trans.scale.ToVECTOR());
	MATRIX offsetRotMat = MGetRotY(DX_PI_F / 2.0f);
	MATRIX offsetPosMat = MGetTranslate((trans.pos + OFFSET).ToVECTOR());

	// 回転行列の合成
	// スケールの行列を剣と合成
	MATRIX localMat = MMult(scaleMat, offsetRotMat);
	// 武器のローカル位置の変換行列を合成
	localMat = MMult(localMat, offsetPosMat);
	// 親子の回転行列を合成(子:武器, 親:手と指定すると親⇒子の順に適用される)
	weaponMatrix_ = MMult(localMat, handMatrix);
	MV1SetMatrix(trans.model, weaponMatrix_);

}

void Kogetsu::WeaponDraw(void)
{
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHand");
	auto framePos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);
	auto handMatrix = MV1GetFrameLocalWorldMatrix(ownerTrans_->get().model, frameIndex);
	auto vec = VScale({ 0.0f,0.1f,0.0f }, 1.0 / 0.025);
	auto swordStartPos = VTransform(vec, handMatrix);

	VECTOR xAxis =
	{
		handMatrix.m[0][0],
		handMatrix.m[0][1],
		handMatrix.m[0][2]
	};

	VECTOR yAxis =
	{
		handMatrix.m[1][0],
		handMatrix.m[1][1],
		handMatrix.m[1][2]
	};

	VECTOR zAxis =
	{
		handMatrix.m[2][0],
		handMatrix.m[2][1],
		handMatrix.m[2][2]
	};

	DrawLine3D(
		swordStartPos,
		VAdd(swordStartPos, VScale(xAxis, 20.0f)),
		GetColor(255, 0, 0));

	DrawLine3D(
		swordStartPos,
		VAdd(swordStartPos, VScale(yAxis, 20.0f)),
		GetColor(0, 255, 0));

	DrawLine3D(
		swordStartPos,
		VAdd(swordStartPos, VScale(zAxis, 20.0f)),
		GetColor(0, 0, 255));

}

void Kogetsu::WeaponAlphaDraw(void)
{
}

void Kogetsu::WeaponUiDraw(void)
{
}

void Kogetsu::WeaponRelease(void)
{
}
