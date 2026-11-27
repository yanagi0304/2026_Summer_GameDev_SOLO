#pragma once

#include "../SceneBase.h"

#include "../../Common/Vector2I.h"

#include "../../Utility/Utility.h"

#include "../../Object/Common/Shader/HologramImageShader.h"
#include "../../Object/Common/Shader/HologramShader.h"

#include "../../Object/Common/Transform/Transform.h"

#include "../../Object/Common/AnimationController/AnimationController.h"

struct ImageData
{
	// 背景画像のハンドル
	int handle;
	// 画像の表示位置
	Vector2I pos;
	// 表示サイズ
	Vector2I size;	
	// 画像サイズ
	float scale;
	// 画像の回転角度
	float rotate;

	// コンストラクタ
	ImageData() : handle(-1), pos{ 0, 0 }, size{ 0, 0 }, scale(1.0f), rotate(0.0f) {}

	// 読み込み
	void Load(const char* path) { handle = LoadGraph(path); }

	// 描画
	void Draw(void) const {
		DrawExtendGraph(pos.x, pos.y, pos.x + size.x, pos.y + size.y, handle, true);
	}
	void DrawRota(void) const {
		DrawRotaGraph(pos.x, pos.y, scale, rotate, handle, true);
	}

	// 解放
	void Release(void) {
		if (handle != -1) {
			DeleteGraph(handle);
			handle = -1;
		}
	}
}; 


class TitleScene : public SceneBase
{
public:
	TitleScene();
	~TitleScene()override = default;

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

	// 解放
	void SubPostRelease(void)override;

#pragma endregion

	// 当たり判定管理は使用しない
	bool UseCollisionManager(void)const override { return false; }


#pragma region メンバ変数

	// 背景画像のデータ
	ImageData background;

	// タイトルロゴのデータ
	ImageData titleLogo;

	// ホログラムシェーダー
	HologramImageShader hologramImageShader;
	HologramShader hologramShader;

	// モデル情報
	Transform trans;

	// モデルのアニメーション管理
	AnimationController* anime;

#pragma endregion

	
};