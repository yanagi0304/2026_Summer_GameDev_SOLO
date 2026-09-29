#include "Shield.h"

Shield::Shield()
{
}

Shield::Shield(const Transform& ownerTrans)
	: WeaponBase("Data/Parameter/Weapon/Shield/")
{
	ownerTrans_ = ownerTrans;
}

void Shield::SubLoad(void)
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
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:Spine2");
	auto framePos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);
	float angle = ownerTrans_->get().angle.y;

	Vector3 forward(
		sinf(angle),
		0.0f,
		cosf(angle));

	trans.pos =
		Vector3(framePos) + OFFSET -
		forward * 80;

	trans.angle = ownerTrans_->get().angle;

}
