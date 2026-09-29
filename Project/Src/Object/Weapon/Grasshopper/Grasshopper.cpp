#include "Grasshopper.h"

Grasshopper::Grasshopper()
{
}

Grasshopper::Grasshopper(const Transform& ownerTrans)
	:
	WeaponBase("Data/Parameter/Weapon/Grasshopper/")
{
	ownerTrans_ = ownerTrans;
}

void Grasshopper::WeaponLoad(void)
{
	trans.Load("Weapon/Grasshopper");
	WeaponInit();

	SetJudge(true);
	SetDynamicFlg(true);
	SetPushFlg(true);
}

void Grasshopper::WeaponInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
}

void Grasshopper::WeaponUpdate(void)
{
	MV1SetOpacityRate(trans.model, 0.4f);
}

void Grasshopper::WeaponDraw(void)
{
}

void Grasshopper::WeaponAlphaDraw(void)
{
}

void Grasshopper::WeaponUiDraw(void)
{
}

void Grasshopper::WeaponRelease(void)
{
}
