#include "StageTutrial.h"
#include "../../Common/Collider/BoxCollider.h"

StageTutrial::StageTutrial():
	ActorBase("Data/Parameter/Stage/BattleStage/")
{
}

void StageTutrial::Load(void)
{
	trans.Load("Stage/Doujou");
	SetJudge(true);
	SetDynamicFlg(false);
	ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "1")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "2")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "3")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "4")));
	//ColliderCreate(new BoxCollider(COLLIDER_TAG::STAGE, GetParameterToVector3("Collider", "5")));
	
}

void StageTutrial::SubInit(void)
{
}
