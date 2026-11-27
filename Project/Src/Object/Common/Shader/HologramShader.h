#pragma once

#include <map>

#include "ShaderBase.h"

#include "HologramParam.h"
#include "HologramConstantBuffer.h"

// モデルをホログラム表示するシェーダー
// 頂点タイプはメッシュごとに自動判別する
class HologramShader : public ShaderBase
{
public:

	explicit HologramShader(const HologramParam& param = HologramParam::Model());
	~HologramShader()override = default;

	// シェーダーの初期化
	void Init(void)override;

	// シェーダーの更新
	void Update(void)override;

	// シェーダー有効リソースの解放
	void Release(void)override;

	// 描画設定はDrawModel内で完結するため、ここでは何もしない
	void Apply(void)override {}
	void ResetApply(void)override {}

	// アクターのモデルを描画
	void DrawModel(Transform& trans)override;

	// モデルハンドルを指定して描画(アクター以外で使う場合)
	void Draw(int model);

	// 調整値の参照(実行中に書き換え可能)
	HologramParam& GetParam(void) { return param; }

	// 指定マテリアルだけ元テクスチャの割合を変える(顔だけ色を残す場合など)
	void SetMaterialTextureRate(int materialIndex, float rate) { materialTextureRate[materialIndex] = rate; }

private:

	// 全メッシュを1回描画する
	void DrawMeshes(int model, bool depthOnly);

	// 調整値
	HologramParam param;

	// マテリアルごとの元テクスチャの割合
	std::map<int, float> materialTextureRate;

	// 定数バッファ
	HologramConstantBuffer constantBuffer;

	// 経過時間(秒)
	float time;
};
