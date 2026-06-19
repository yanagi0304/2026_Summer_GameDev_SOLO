#include "Asteroid.h"
#include "../../Common/Collider/BoxCollider.h"

Asteroid::Asteroid()
	:WeaponBase("Data/Parameter/Weapon/Asteroid/")
{
}

Asteroid::Asteroid(const Transform& ownerTrans)
	:WeaponBase("Data/Parameter/Weapon/Asteroid/")
{
	ownerTrans_ = ownerTrans;
}

void Asteroid::WeaponLoad(void)
{
	trans.Load("Weapon/Asteroid");
	WeaponInit();

	SetJudge(true);
	SetDynamicFlg(true);
	SetPushFlg(true);

	//ColliderCreate(new BoxCollider(COLLIDER_TAG::ASTEROID, GetParameterToVector3("Collider", "Size"),-1,Vector3(0.0f,30.0f,0.0f)));
}

void Asteroid::WeaponInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
}

void Asteroid::WeaponUpdate(void)
{
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHandIndex2");
	trans.pos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);
}

void Asteroid::WeaponDraw(void)
{
}

void Asteroid::WeaponAlphaDraw(void)
{
}

void Asteroid::WeaponUiDraw(void)
{
}

void Asteroid::WeaponRelease(void)
{
}
