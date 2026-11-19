#pragma once

#include <memory>

#include "../SceneBase.h"

#include "../../Object/Common/MeshSlicer/CutMesh.h"
#include "../../Object/Common/CuttableActor/CuttableActor.h"
#include "../../Object/Enemy/Navigation/NavGrid.h"
#include "../../Object/Enemy/Navigation/AStar.h"
#include "../../Object/Enemy/Navigation/NavManager.h"
#include "../../Object/Enemy/EnemyManager.h"
#include "../../Object/Jazz/TrionGauge.h"

class GameScene : public SceneBase
{
public:
	GameScene();
	~GameScene()override = default;

private:

#pragma region 主要関数再定義

	// 読み込み
	void SubPostLoad(void)override;

	// 初期化
	void SubPostInit(void)override;

	// 更新
	void SubPostUpdate(void)override;

	// 描画(メイン処理の後)
	void SubPostDraw(void)override;

	// UI描画
	void SubUiDraw(void)override;

#pragma endregion

	bool UseSkyBox(void)const override { return true; }

	void CreateCamera(void)override;

	int CountMannequins(void) const;

	// 敵の攻撃がプレイヤーに当たった時(トリオンが漏れて、少し吹き飛ぶ)
	void OnPlayerHit(const Vector3& enemyPos);

	// トリオンゲージの表示
	void DrawTrionGauge(void) const;

	// トリオン(プレイヤーの体力)。尽きたらゲームオーバー
	TrionGauge trion;

	//操縦オブジェ
	ActorBase* operatorObject;

	//マネキン
	ActorBase* mannequinManager;

	//debug
	CutMesh front;
	CutMesh back;

	CuttableActor* box_; 
	bool prevC_;

	// ---- 経路探索デバッグ(F3で表示切り替え) ----
	// グリッドを作り、敵の出現(ウェーブ)を管理する
	void NavDebugInit(void);
	void NavDebugUpdate(void);
	void NavDebugDraw(void) const;

	NavGrid navGrid;
	NavManager navManager{ navGrid };	// 経路探索の窓口(1フレームの探索数を制限する)
	std::unique_ptr<EnemyManager> enemyManager;
	bool navShow = false;
	bool navKeyPrev = false;
	bool defeatKeyPrev = false;


};