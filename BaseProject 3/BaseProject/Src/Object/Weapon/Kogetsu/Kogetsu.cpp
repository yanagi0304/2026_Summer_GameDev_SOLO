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

void Kogetsu::WeaponLoad(void)
{
	trans.Load("Weapon/Kogetsu");
	WeaponInit();

	SetJudge(true);
	SetDynamicFlg(true);

	ColliderCreate(new CapsuleCollider(COLLIDER_TAG::KOGETSU, GetParameter("Collider", "ColliderStartPos")
	,GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));
}

void Kogetsu::WeaponInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = Vector3(GetParameter("Init", "scale"));

}

void Kogetsu::WeaponUpdate(void)
{
	trans.pos = MV1GetFramePosition(ownerTrans_->get().model, 22);
	trans.pos.y += GetParameter("Init", "modelOffset");

	//MATRIX handMatrix = MV1GetFrameLocalWorldMatrix(ownerTrans_->get().model, 22);

	//MatrixCombineParentChild(trans.model, trans.pos, { trans.angle }, handMatrix);

}

void Kogetsu::WeaponDraw(void)
{
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
