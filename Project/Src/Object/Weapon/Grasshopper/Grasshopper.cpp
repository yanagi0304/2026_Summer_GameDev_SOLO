#include "Grasshopper.h"

#include "../../../Utility/Utility.h"

Grasshopper::Grasshopper()
	:
	WeaponBase(Transform(), "Data/Parameter/Weapon/Grasshopper/")
{
}

Grasshopper::Grasshopper(const Transform& ownerTrans)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Grasshopper/")
{
}

void Grasshopper::Load(void)
{
	trans.LoadModel("Weapon/Grasshopper");

	SetJudgeFlg(true);
	SetDynamicFlg(true);
	SetPushFlg(true);
}

void Grasshopper::SubInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
}

void Grasshopper::SubUpdate(void)
{
	MV1SetOpacityRate(trans.model, 0.4f);

	// オーナーの回転に合わせてローカル位置を回転
	trans.pos =
		ownerTrans_.pos +
		ownerTrans_.VTrans(
			Vector3::YZonly(50.0f, 150.0f)
		);

	// オーナーの姿勢を基準に、ローカルX軸へ-40度回転
	const Quaternion localRotation =
		Quaternion::FromRotationX(
			Deg2Rad(-40.0f)
		);

	// オーナーの回転 + Grasshopper固有の回転
	const MATRIX rotationMat =
		MMult(
			localRotation.ToMatrix(),
			ownerTrans_.RotationMat()
		);

	trans.SetRotation(
		Quaternion::FromMatrix(rotationMat)
	);
}

void Grasshopper::SubDraw(void)
{
}

void Grasshopper::SubRelease(void)
{
}