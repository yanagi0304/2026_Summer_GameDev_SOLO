#include "StageTutrial.h"
#include "../../Common/Collider/MeshCollider.h"

StageTutrial::StageTutrial():
	ActorBase("Data/Parameter/Stage/TutrialStage/")
{
}

void StageTutrial::Load(void)
{
	trans.LoadModel("Stage/demoStage");
	// 動的オブジェクトとしての処理を有効にする
	SetDynamicFlg(false);

	// 重力を有効にする
	SetGravityFlg(true);

	// 当たり判定による押し出しを有効にする
	SetPushFlg(true);
	AddCollider(new MeshCollider(COLLIDER_TAG::DebugObject));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "2")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "3")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "4")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "5")));
	
}

void StageTutrial::SubInit(void)
{
	trans.scale = GetParameterToVector3("Init", "1");
}
