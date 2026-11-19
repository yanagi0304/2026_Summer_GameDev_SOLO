#include "GameScene.h"

#include "../../Utility/Utility.h"

#include "../../Application/Application.h"

#include "../../Manager/TimeScale/TimeScale.h"
#include "../../Manager/Input/InputManager.h"
#include "../../Manager/Sound/SoundManager.h"
#include "../../Manager/Font/FontManager.h"

#include "../../Manager/Camera/ShoulderCamera/ShoulderCamera.h"

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
#include "../../Object/Enemy/Enemy.h"

#include "../../Object/Common/CuttableActor/CuttableActor.h"

#include "../../Object/Common/MeshSlicer/CutRequestQueue.h"
#include "../../Object/Mannequin/Mannequin.h"


GameScene::GameScene() : SceneBase()
{
}

void GameScene::SubPostLoad(void)
{
	Snd::GetIns().ChangeScene("Game");

	//AddActor(new BoxDebugObject(Vector3(20000, 1000, 20000), Vector3::Yonly(-500), false));

	
	operatorObject = new Jazz();
	AddActor(operatorObject);
	mannequinManager = new MannequinManager(operatorObject->GetTrans().pos);
	AddActor(mannequinManager);
	AddActor(new StageTutrial);

	// デバッグ用の敵(経路探索の確認用。出現位置はステージに合わせて調整する)
	debugEnemy = new Enemy(Enemy::NavContext{ &navGrid, &navAStar }, &operatorObject->GetTrans().pos, Vector3(1500.0f, 100.0f, 0.0f));
	AddActor(debugEnemy);

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
	NavDebugInit();
	//AddPostEffect(new FocusLinesPostEffect(1.5f, 18.0f, 100.0f));
	
}

void GameScene::SubPostUpdate(void)
{
	NavDebugUpdate();

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
}

void GameScene::SubPostDraw(void)
{
	NavDebugDraw();
}

void GameScene::SubUiDraw(void)
{
	if (navShow)
	{
		DrawFormatString(0, 60, 0xffffff, "[F3] NavDebug: red=blocked cell / cyan=enemy path / green line=enemy sees you");
	}
	DrawStringToHandle(0, 0, "ゲーム", 0xffffff, Font::GetIns().GetFont(FontKinds::Marumiya40));
}

void GameScene::CreateCamera(void)
{
	camera = new ShoulderCamera(&ActorSerch<Jazz>(actors)->GetTrans().pos);
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
// ---- 経路探索デバッグ ----

void GameScene::NavDebugInit(void)
{
	// ステージの範囲をグリッドの対象にする
	StageTutrial* stage = ActorSerch<StageTutrial>(actors);
	if (stage == nullptr) { return; }

	const MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(stage->GetTrans().model, -1, TRUE, TRUE);
	if (ref.PolygonNum <= 0) { return; }

	const Vector3 minPos(ref.MinPosition);
	const Vector3 maxPos(ref.MaxPosition);
	const Vector3 center = (minPos + maxPos) * 0.5f;
	const float sizeX = maxPos.x - minPos.x;
	const float sizeZ = maxPos.z - minPos.z;

	// 単位はcm。プレイヤーのコライダー(半径40、全高220)に合わせる
	NavGrid::Config cfg;
	cfg.minPos = minPos;
	cfg.maxPos = maxPos;
	cfg.floorY = minPos.y;
	cfg.cellSize = (std::max)((std::max)(sizeX, sizeZ) / 100.0f, 100.0f);   // 最大100x100セル
	cfg.agentRadius = 45.0f;
	cfg.agentHeight = 220.0f;
	cfg.stepHeight = 30.0f;

	// 障害物(ビル)はCSVから読む。無ければ仮のビルを置く
	// CSVの書式: centerX,centerY,centerZ,halfX,halfY,halfZ,rotYDeg (#でコメント)
	const int loaded = navGrid.LoadObstaclesCsv("Data/Navigation/TutrialStage_Obstacles.csv");
	if (loaded <= 0)
	{
		// 仮のビル(位置は範囲に対する割合で決める)
		const float h = 1000.0f;
		navGrid.AddObstacleBox(Vector3(center.x - sizeX * 0.20f, cfg.floorY + h * 0.5f, center.z), Vector3(sizeX * 0.04f, h * 0.5f, sizeZ * 0.18f), 0.0f);
		navGrid.AddObstacleBox(Vector3(center.x + sizeX * 0.15f, cfg.floorY + h * 0.5f, center.z + sizeZ * 0.10f), Vector3(sizeX * 0.10f, h * 0.5f, sizeZ * 0.04f), 30.0f);
		navGrid.AddObstacleBox(Vector3(center.x, cfg.floorY + h * 0.5f, center.z - sizeZ * 0.25f), Vector3(sizeX * 0.08f, h * 0.5f, sizeZ * 0.08f), 0.0f);
	}
	navGrid.Build(cfg);
}

void GameScene::NavDebugUpdate(void)
{
	// F3で表示切り替え
	const bool keyNow = (CheckHitKey(KEY_INPUT_F3) == 1);
	if (keyNow && !navKeyPrev) { navShow = !navShow; }
	navKeyPrev = keyNow;
}

void GameScene::NavDebugDraw(void) const
{
	if (!navShow || !navGrid.IsBuilt()) { return; }

	navGrid.DrawDebug(false);

	if (debugEnemy != nullptr) { debugEnemy->DrawDebug(); }
}