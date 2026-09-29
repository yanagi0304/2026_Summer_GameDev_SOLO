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
	SetDynamicFlg(true);
	SetGravityFlg(true);

	ColliderCreate(new CapsuleCollider(COLLIDER_TAG::MANNEQUIN, GetParameterToVector3("Collider", "ColliderStartPos")
		, GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));
}

void Mannequin::SubInit(void)
{
	trans.scale = GetParameter("Init", "Scale");

	int vertexCount = MV1GetMeshVertexNum(trans.model, 0);

	trans.pos = (0.0, 0.0, 200.0f);

}
