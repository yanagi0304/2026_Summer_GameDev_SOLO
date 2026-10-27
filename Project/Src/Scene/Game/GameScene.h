#pragma once

#include"../SceneBase.h"

#include "../../Object/Common/MeshSlicer/CutMesh.h"
#include "../../Object/Common/CuttableActor/CuttableActor.h"

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


};