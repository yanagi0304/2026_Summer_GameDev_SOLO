#pragma once

#include "ShaderBase.h"

#include "../../../Common/Vector3.h"

// 煙が弾ける演出のシェーダー
// 粒の動きは頂点シェーダー(SmokeBurstVS)で計算し、このクラスは時間と調整値を渡す
// 使い方: アクターで CreateShader(new SmokeBurstShader(位置, 向き, 乱数)) を呼ぶ
class SmokeBurstShader : public ShaderBase
{
public:

	// 煙が消えるまでの基準時間(秒)
	static constexpr float DURATION = 0.7f;

	// 粒の発生の遅れの最大(秒)
	static constexpr float MAX_DELAY = 0.08f;

	// 最後の粒が消えるまでの時間(秒)
	static constexpr float TOTAL_TIME = DURATION + MAX_DELAY;

	// origin: 煙の発生位置 / normal: 煙が広がる側の向き / seed: 発生ごとの乱数(0〜1)
	SmokeBurstShader(const Vector3& origin, const Vector3& normal, float seed);
	~SmokeBurstShader() override = default;

	// シェーダーの初期化
	void Init(void) override;

	// シェーダーの更新(経過時間を進める)
	void Update(void) override;

	// シェーダーを描画に適用
	void Apply(void) override;

	// シェーダーの設定リセット
	void ResetApply(void) override;

	// このシェーダーの終了判定(最後の粒が消えたらtrue)
	bool IsEnd(void) const override;

	// シェーダー固有リソースの解放
	void Release(void) override;

private:

	// 頂点シェーダー用の定数バッファ(SmokeBurstVSのb4と同じ並び)
	struct alignas(16) VertexParam
	{
		// 発生位置
		float origin[3];
		// 経過時間(秒)
		float time;

		// 煙が広がる側の向き
		float hitNormal[3];
		// 消えるまでの基準時間(秒)
		float duration;

		// 粒の初速
		float burstSpeed;
		// 空気抵抗
		float drag;
		// 上へ浮かぶ強さ
		float rise;
		// 発生直後の粒の大きさ
		float sizeStart;

		// 消える頃の粒の大きさ
		float sizeEnd;
		// 発生ごとの乱数(0〜1)
		float seed;
		float padding[2];
	};

	// ピクセルシェーダー用の定数バッファ(SmokeBurstPSのb4と同じ並び)
	struct alignas(16) PixelParam
	{
		// 明るい側の煙の色(RGBA)
		float lightColor[4];
		// 影側の煙の色(RGBA)
		float shadowColor[4];

		// 輪郭をノイズで崩す強さ
		float edgeNoise;
		// 時間とともに煙が削れていく強さ
		float erosion;
		float padding[2];
	};

	// 頂点シェーダー用の値
	VertexParam vertexParam;

	// ピクセルシェーダー用の値
	PixelParam pixelParam;

	// 頂点シェーダー用の定数バッファハンドル
	int vertexConstantBuffer;

	// ピクセルシェーダー用の定数バッファハンドル
	int pixelConstantBuffer;

	// 発生してからの経過時間(秒)
	float time;
};
