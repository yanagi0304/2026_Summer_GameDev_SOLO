#include "Shield.h"

Shield::Shield()
	: WeaponBase(Transform(), "Data/Parameter/Weapon/Shield/")
{
}

Shield::Shield(const Transform& ownerTrans)
	: WeaponBase(ownerTrans, "Data/Parameter/Weapon/Shield/")
{
}

void Shield::Load(void)
{
	trans.LoadModel("Weapon/Shield");

	SetJudgeFlg(true);
	SetDynamicFlg(true);
	SetPushFlg(true);
}

void Shield::SubInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
}

void Shield::SubUpdate(void)
{
	MV1SetOpacityRate(trans.model, 0.4f);

	const auto frameIndex =
		MV1SearchFrame(
			ownerTrans_.model,
			"mixamorig:Spine2"
		);

	const Vector3 framePos =
		MV1GetFramePosition(
			ownerTrans_.model,
			frameIndex
		);

	// オーナーの前方向
	const Vector3 forward =
		ownerTrans_.VTrans(
			Vector3(0.0f, 0.0f, 1.0f)
		);

	// 座標
	trans.pos =
		framePos +
		OFFSET +
		forward * 80.0f;

	// オーナーと同じ姿勢にする
	trans.SetRotation(ownerTrans_.rotation);
}
