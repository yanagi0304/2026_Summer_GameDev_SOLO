#pragma once

#include"../SceneBase.h"

#include "../../Object/Common/MeshSlicer/CutMesh.h"
#include "../../Object/Common/CuttableActor/CuttableActor.h"
#include "../../Object/Enemy/Navigation/NavGrid.h"
#include "../../Object/Enemy/Navigation/AStar.h"

class Enemy;

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
	// グリッドを作り、デバッグ用の敵を1体出す
	void NavDebugInit(void);
	void NavDebugUpdate(void);
	void NavDebugDraw(void) const;

	NavGrid navGrid;
	AStar navAStar;
	Enemy* debugEnemy = nullptr;	// シーンが所有する(ここでは参照のみ)
	bool navShow = false;
	bool navKeyPrev = false;


};