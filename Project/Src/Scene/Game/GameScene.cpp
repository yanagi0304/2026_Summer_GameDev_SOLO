#include "GameScene.h"

#include "../../Utility/Utility.h"

#include "../../Application/Application.h"

#include "../../Manager/TimeScale/TimeScale.h"
#include "../../Manager/Input/InputManager.h"
#include "../../Manager/Sound/SoundManager.h"
#include "../../Manager/Font/FontManager.h"

#include "../../Manager/Camera/FollowRemote/FollowRemoteCamera.h"

#include "../SceneManager.h"

#include "../ActorUseDefine.h"

#include "../Common/PostEffect/CRTPostEffect/CRTPostEffect.h"
#include "../Common/PostEffect/FocusLinesPostEffect/FocusLinesPostEffect.h"

#include "../../Object/Common/DebugObject/BoxDebugObject.h"
#include "../../Object/Common/DebugObject/SphereDebugObject.h"
#include "../../Object/Common/DebugObject/CapsuleDebugObject.h"
#include "../../Object/Common/DebugObject/MeshDebugObject.h"

#include "../../Object/Jazz/Jazz.h"
#include "../../Object/Mannequin/MannequinManager.h"
#include "../../Object/Stage/StageTutrial/StageTutrial.h"

#include "../../Object/Water.h"

#include "../../Object/Common/CuttableActor/CuttableActor.h"

#include "../../Object/Common/MeshSlicer/CutRequestQueue.h"
#include "../../Object/Mannequin/Mannequin.h"

#include "../../Object/Common/Target/Target.h"

GameScene::GameScene() : SceneBase()
{
}

void GameScene::SubPostLoad(void)
{
	Snd::GetIns().ChangeScene("Game");

	//AddActor(new BoxDebugObject(Vector3(20000, 1000, 20000), Vector3::Yonly(-500), false));

	mannequinManager = new MannequinManager();
	AddActor(mannequinManager);
	operatorObject = new Jazz([&]() {return target->GetTargetPos(); });
	AddActor(operatorObject);
	AddActor(new StageTutrial);

	//int debugObjNum = 50;
	//float debugObjSpace = 10.0f;
	//for (int i = 0; i < debugObjNum; i++) {
	//	AddActor(new SphereDebugObject(50.0f, Vector3::Xonly(debugObjSpace * (float)(debugObjNum / 2) - ((float)debugObjSpace * i))));
	//	AddActor(new SphereDebugObject(50.0f, Vector3::Zonly(debugObjSpace * (float)(debugObjNum / 2) - ((float)debugObjSpace * i))));
	//}

	// GameScene::SubPostLoad
	//CutRequestQueue::Clear();
	//auto* box = new CuttableActor(COLLIDER_TAG::Stage, false);
	//box->SetMesh(CutMesh::MakeBox(Vector3(200, 200, 200)));
	//box->SetPos(Vector3(0, 100, 200));      // Jazz の前。位置は調整してください
	//AddActor(box);

}

void GameScene::SubPostInit(void)
{
	//AddPostEffect(new FocusLinesPostEffect(1.5f, 18.0f, 100.0f));
	
}

void GameScene::SubPostUpdate(void)
{

	for (const CutRequest& request : CutRequestQueue::Drain()) {
		CuttableActor* fragment = request.target->Cut(request.plane, request.cutId);
		if (fragment != nullptr) { AddActor(fragment); }
	}

	std::vector<ActorBase*> removeList;

	for (ActorBase* a : actors) {
		CuttableActor* c = dynamic_cast<CuttableActor*>(a);
		if (c != nullptr && c->IsVanished()) { removeList.push_back(a); }
	}

	for (ActorBase* a : removeList) {
		RemoveActor(a);
	}

	// ゲーム終了処理
	if (Input::GetIns().GetInfo(KEY_TYPE::End).down) {
		SceneManager::GetIns().ChangeSceneFade(SCENE_ID::Title);
	}

	// 決定
	if (CheckHitKey(KEY_INPUT_RSHIFT) == 1 || CountMannequins() == 0) {
		SceneManager::GetIns().ChangeSceneFade(SCENE_ID::GameClear);
	}

	target->Update();
}

void GameScene::SubUiDraw(void)
{
	DrawStringToHandle(0, 0, "ゲーム", 0xffffff, Font::GetIns().GetFont(FontKinds::Marumiya40));
}

void GameScene::CreateCamera(void)
{
	camera = new FollowRemoteCamera(&ActorSerch<Jazz>(actors)->GetTrans().pos,
		Vector3(0.0,500.0f,-300.0f),Vector3::Yonly(200.0f));
}

int GameScene::CountMannequins(void) const
{
	int count = 0;

	for (ActorBase* a : actors) {

		const MannequinManager* m = dynamic_cast<const MannequinManager*>(a);
		if (m == nullptr) { continue; }
		for (auto* b : m->GetChildActors()) {

			const Mannequin* c = dynamic_cast<const Mannequin*>(b);

			// 縮み始めた・消えたものは「もういない」扱いにする
			if (c->IsVanished()) { continue; }

			count++;
		}
		
	}

	return count;
}
