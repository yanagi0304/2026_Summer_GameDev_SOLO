#pragma once

#include "HologramParam.h"
#include "HologramConstantBuffer.h"

// 画像をホログラム表示するシェーダー
class HologramImageShader
{
public:

	explicit HologramImageShader(const HologramParam& param = HologramParam::Image());
	~HologramImageShader() = default;

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// 解放
	void Release(void);

	// 左上座標とサイズを指定して描画
	void Draw(int x, int y, int width, int height, int graphHandle);

	// 中心座標・拡大率・回転角(ラジアン)を指定して描画
	void DrawRota(int x, int y, float scale, float angle, int graphHandle);

	// 調整値の参照(実行中に書き換え可能)
	HologramParam& GetParam(void) { return param; }

private:

	// 中心座標と表示サイズから四角形を描画する
	void DrawQuad(float centerX, float centerY, float width, float height, float angle, int graphHandle);

	// 調整値
	HologramParam param;

	// 定数バッファ
	HologramConstantBuffer constantBuffer;

	// ピクセルシェーダーのハンドル
	int pixelShaderHandle;

	// 経過時間(秒)
	float time;
};
