#include "Mannequin.h"
#include "../Common/Collider/CapsuleCollider.h"

Mannequin::Mannequin()
	:ActorBase("Data/Parameter/Mannequin/")
{
}

void Mannequin::Load(void)
{
	trans.Load("Mannequin/mannequin");
	SetJudge(true);
	SetDynamicFlg(false);
	ColliderCreate(new CapsuleCollider(COLLIDER_TAG::MANNEQUIN, GetParameterToVector3("Collider", "ColliderStartPos")
		, GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));
}

void Mannequin::SubInit(void)
{
	trans.scale = GetParameter("Init", "Scale");

}
