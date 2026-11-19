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


namespace
{
	// グリッド生成にかかった時間(ミリ秒。デバッグ表示用)
	double g_navBuildMs = 0.0;
}

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

	// 敵の出現・ウェーブ管理(シーンのActor追加・削除を関数で渡す)
	enemyManager = std::make_unique<EnemyManager>(
		navGrid, navManager, &operatorObject->GetTrans().pos,
		[this](ActorBase* actor) { AddActor(actor); },
		[this](ActorBase* actor) { RemoveActor(actor); });

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

	// 敵が出した探索依頼を処理し(1フレームの件数に上限あり)、敵の出現・撤去を進める
	navManager.Update();
	if (enemyManager) { enemyManager->Update(); }

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
		DrawFormatString(0, 80, 0xffffff, "[F4] defeat all enemies (debug)");

		if (enemyManager)
		{
			const WaveController& wave = enemyManager->GetWave();
			if (wave.IsFinished())
			{
				DrawString(0, 100, "ALL WAVES CLEARED", 0xffff00);
			}
			else
			{
				DrawFormatString(0, 100, 0xffffff, "WAVE %d/%d  alive:%d  to-spawn:%d  rest:%.1fs",
					wave.GetWaveNumber(), wave.GetWaveTotal(), enemyManager->GetAliveCount(), wave.GetRemainingToSpawn(),
					(wave.GetPhase() == WaveController::Phase::Resting) ? wave.GetRestRemainingFrames() / 60.0f : 0.0f);
			}
		}

		DrawFormatString(0, 120, 0xffffff, "NavQueue: pending=%d processed=%d/frame search=%.3fms",
			navManager.GetPendingCount(), navManager.GetLastProcessedCount(), navManager.GetLastUpdateMs());

		// 重さの切り分け用(出現1回にかかった時間と、その中のモデル準備の時間)
		DrawFormatString(0, 140, 0xffffff, "FPS:%.0f  lastSpawn=%.1fms (model=%.1fms)  grid walkable=%d/%d (build %.0fms)",
			GetFPS(), enemyManager ? enemyManager->GetLastSpawnMs() : 0.0, Enemy::GetLastModelLoadMs(),
			navGrid.CountWalkable(), navGrid.GetWidth() * navGrid.GetHeight(), g_navBuildMs);

		// グリッドの細かさと、対象の範囲(m)
		DrawFormatString(0, 160, 0xffffff, "grid %dx%d  cell=%.0fcm  area=%.0fm x %.0fm",
			navGrid.GetWidth(), navGrid.GetHeight(), navGrid.GetCellSize(),
			(navGrid.GetConfig().maxPos.x - navGrid.GetConfig().minPos.x) / 100.0f, (navGrid.GetConfig().maxPos.z - navGrid.GetConfig().minPos.z) / 100.0f);
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
	const LONGLONG buildStart = GetNowHiPerformanceCount();

	// ステージの範囲をグリッドの対象にする
	StageTutrial* stage = ActorSerch<StageTutrial>(actors);
	if (stage == nullptr || operatorObject == nullptr) { return; }

	const MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(stage->GetTrans().model, -1, TRUE, TRUE);
	if (ref.PolygonNum <= 0) { return; }

	const Vector3 minPos(ref.MinPosition);
	const Vector3 maxPos(ref.MaxPosition);
	const Vector3 playerPos = operatorObject->GetTrans().pos;
	const float sizeX = maxPos.x - minPos.x;
	const float sizeZ = maxPos.z - minPos.z;

	// 単位はcm。プレイヤーのコライダー(半径40、全高220)に合わせる
	NavGrid::Config cfg;
	cfg.minPos = minPos;
	cfg.maxPos = maxPos;
	cfg.floorY = playerPos.y;   // 床は平らな前提で、プレイヤーの足元の高さを床とする
	cfg.cellSize = (std::max)((std::max)(sizeX, sizeZ) / 100.0f, 100.0f);   // 最大100x100セル
	cfg.agentRadius = 45.0f;
	cfg.agentHeight = 220.0f;
	cfg.stepHeight = 30.0f;

	// プレイヤーのいる場所から歩いて行けないセル(建物の内部など)は、歩行不可にする
	cfg.keepOnlyReachable = true;
	cfg.reachableSeed = playerPos;

	// ステージのメッシュを障害物として登録する(壁・ビルの側面だけがBuildで使われる)
	for (int i = 0; i < ref.PolygonNum; ++i)
	{
		const MV1_REF_POLYGON& polygon = ref.Polygons[i];
		navGrid.AddObstacleTriangle(
			Vector3(ref.Vertexs[polygon.VIndex[0]].Position),
			Vector3(ref.Vertexs[polygon.VIndex[1]].Position),
			Vector3(ref.Vertexs[polygon.VIndex[2]].Position));
	}

	// メッシュに無い障害物を手で足したい時は、CSVにボックスを書く(無ければ何もしない)
	// CSVの書式: centerX,centerY,centerZ,halfX,halfY,halfZ,rotYDeg (#でコメント)
	navGrid.LoadObstaclesCsv("Data/Navigation/TutrialStage_Obstacles.csv");

	navGrid.Build(cfg);

	g_navBuildMs = (GetNowHiPerformanceCount() - buildStart) / 1000.0;
}
void GameScene::NavDebugUpdate(void)
{
	// F3で表示切り替え
	const bool keyNow = (CheckHitKey(KEY_INPUT_F3) == 1);
	if (keyNow && !navKeyPrev) { navShow = !navShow; }
	navKeyPrev = keyNow;

	// F4で、いる敵を全員倒した状態にする(ウェーブの進行確認用)
	const bool defeatKeyNow = (CheckHitKey(KEY_INPUT_F4) == 1);
	if (defeatKeyNow && !defeatKeyPrev && enemyManager) { enemyManager->DefeatAll(); }
	defeatKeyPrev = defeatKeyNow;
}

void GameScene::NavDebugDraw(void) const
{
	if (!navShow || !navGrid.IsBuilt()) { return; }

	navGrid.DrawDebug(false);

	if (enemyManager) { enemyManager->DrawDebug(); }
}