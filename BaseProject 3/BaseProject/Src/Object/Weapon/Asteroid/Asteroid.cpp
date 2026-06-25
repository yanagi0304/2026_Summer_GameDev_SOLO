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

	ColliderCreate(new BoxCollider(COLLIDER_TAG::ASTEROID, GetParameterToVector3("Collider", "Size"),-1,Vector3(0.0f,30.0f,0.0f)));
}

void Asteroid::WeaponInit(void)
{
	trans.scale = (SIZE);
	//trans.pos += GetParameter("Init", "modelOffset");
}

void Asteroid::WeaponUpdate(void)
{
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHandIndex2");
	Vector3 hand = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);
	float angle = ownerTrans_->get().angle.y;
	trans.scale = (SIZE);
	Vector3 right(
		cosf(angle),
		0.0f,
		-sinf(angle));

	trans.pos =
		hand -
		right * 80.0f +
		Vector3(0.0f, 60.0f, 0.0f);
}

void Asteroid::WeaponDraw(void)
{
	/*auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHandIndex2");
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
		GetColor(0, 0, 255));*/

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
