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
#include "../../Manager/Collision/CurrentCollisionManager.h"

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

	// ゲームクリア: 全ウェーブの敵を倒したら(右Shiftは確認用の強制クリア)
	const bool allEnemiesDefeated = enemyManager && enemyManager->GetWave().IsFinished();
	if (CheckHitKey(KEY_INPUT_RSHIFT) == 1 || allEnemiesDefeated) {
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

		// グリッド生成の内訳(ミリ秒)
		const NavGrid::BuildTimings& bt = navGrid.GetBuildTimings();
		DrawFormatString(0, 180, 0xffffff, "build detail: floors=%.0f walls=%.0f reachable=%.0f nearest=%.0f", bt.floorsMs, bt.obstaclesMs, bt.reachableMs, bt.nearestMs);

		// プレイヤーの段差判定の結果(なぜ登れた/登れないか)
		if (const CharacterBase* player = dynamic_cast<const CharacterBase*>(operatorObject))
		{
			static const char* const NAMES[] = { "Disabled", "NotPressing", "Progressing", "NotBlockedYet", "NotGrounded", "NoStageObstacle", "WallTooTall", "LimitReached", "Lifting" };
			static StepClimbDebug lastEvent;
			const StepClimbDebug& sd = player->GetStepClimbDebug();

			// 押していない・進めている・まだ数フレーム・空中は、入れ替わりが速いので、それ以外の結果だけ残す
			if (sd.result != StepClimbResult::NotPressing && sd.result != StepClimbResult::Progressing && sd.result != StepClimbResult::NotBlockedYet && sd.result != StepClimbResult::NotGrounded) { lastEvent = sd; }

			DrawFormatString(0, 200, 0xffffff, "Step now : %s  wallDist=%.0f  lifted=%.0f  progress=%.1f  y=%.1f  clearance=%.1f", NAMES[static_cast<int>(sd.result)], sd.wallDistance, sd.lifted, sd.progress, player->GetTrans().pos.y, sd.groundClearance);
			DrawFormatString(0, 220, 0xffff00, "Step last: %s  wallDist=%.0f  lifted=%.0f", NAMES[static_cast<int>(lastEvent.result)], lastEvent.wallDistance, lastEvent.lifted);
			DrawFormatString(0, 280, 0xffffff, "Grounded: flag=%d  nearGround(used for jump)=%d", player->GetGroundFlag() ? 1 : 0, player->IsGroundedLoose() ? 1 : 0);
			// 速度(1フレームあたり)。滑り落ちているかを見るため: 入力なしで地面にいるのに、水平の速度が0にならなければ滑っている
			if (const PhysicsBody* body = player->GetPhysicsBody())
			{
				const Vector3 v = body->GetVelocity();
				DrawFormatString(0, 300, 0xffffff, "Vel: horizontal=%.2f  vertical=%.2f", std::sqrt(v.x * v.x + v.z * v.z), v.y);
			}
		}

		// ステージへのレイの実測値(段差の高さと、レイが機能しているかを調べるため)
		if (operatorObject != nullptr && CurrentCollisionManager::Exists())
		{
			CollisionManager& cm = CurrentCollisionManager::Get();
			const Vector3 pp = operatorObject->GetTrans().pos;
			Vector3 fwd = operatorObject->GetTrans().rotation.Rotate(Vector3(0.0f, 0.0f, 1.0f));
			fwd.y = 0.0f;
			fwd = fwd.LengthSq() > 1.0e-6f ? fwd.Normalized() : Vector3(0.0f, 0.0f, 1.0f);

			RaycastHit hit;
			const bool down = cm.RaycastStage(pp + Vector3(0.0f, 50.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), 250.0f, hit);
			DrawFormatString(0, 240, 0xffffff, "RayDown(stage): %s  groundY=%.1f  playerY=%.1f", down ? "HIT" : "MISS", down ? hit.point.y : 0.0f, pp.y);

			// 進行(正面)方向へ、足元から指定の高さで飛ばした時の、壁までの距離(cm)。当たらなければ--
			// 地面(真下のレイが当たった高さ)を基準にする。当たらなければ足元の高さ
			const float groundY = down ? hit.point.y : pp.y;
			const float heights[5] = { 2.0f, 5.0f, 10.0f, 20.0f, 40.0f };
			char buffer[160];
			int length = snprintf(buffer, sizeof(buffer), "RayFwd(150cm) above GROUND:");
			for (float h : heights)
			{
				const bool ok = cm.RaycastStage(Vector3(pp.x, groundY + h, pp.z), fwd, 150.0f, hit);
				length += snprintf(buffer + length, sizeof(buffer) - length, "  +%.0f:", h);
				length += ok ? snprintf(buffer + length, sizeof(buffer) - length, "%.0f", hit.distance) : snprintf(buffer + length, sizeof(buffer) - length, "--");
			}
			DrawString(0, 260, buffer, 0xffffff);
		}	}
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
	// セルは100cm。壁の余白がセルの半分強なので、幅2m以上の通路は通れる
	// ステージが極端に大きい場合だけ、セルを大きくして最大400x400セルに収める
	cfg.cellSize = (std::max)((std::max)(sizeX, sizeZ) / 400.0f, 100.0f);
	cfg.agentRadius = 45.0f;
	cfg.agentHeight = 220.0f;
	// 縁石や段差: この高さまでの障害物は乗り越えられるものとして無視する(壁にしない)
	// maxFloorStep(隣のセルとの床の高さの差の上限。初期値60)がこれ以上であること
	cfg.stepHeight = 50.0f;
	cfg.maxFloorStep = 60.0f;

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

	// プレイヤーの段差判定のレイ(直下の地面=黄、前方の障害物=水色、背の高い壁の判定=青。当たった点は赤)
	if (const CharacterBase* player = dynamic_cast<const CharacterBase*>(operatorObject))
	{
		static const unsigned int COLORS[5] = { 0xffff00, 0x00ffff, 0x4060ff, 0x00ff00, 0xff00ff };
		const auto& probes = player->GetStepClimbDebug().probes;
		for (size_t i = 0; i < probes.size(); ++i)
		{
			DrawLine3D(probes[i].from.ToVECTOR(), probes[i].to.ToVECTOR(), COLORS[probes[i].kind % 5]);
			if (probes[i].hit) { DrawSphere3D(probes[i].to.ToVECTOR(), 4.0f, 6, 0xff0000, 0xff0000, TRUE); }
		}
	}}