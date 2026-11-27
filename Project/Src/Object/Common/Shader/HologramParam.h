#pragma once

// ホログラム表現の調整値
struct HologramParam
{
	// ホログラムの色(0.0 ～ 1.0)
	float colorR = 0.15f;
	float colorG = 0.85f;
	float colorB = 1.0f;

	float textureRate = 0.15f;	// 元テクスチャの色を残す割合(0 = 単色 / 1 = 元の色)
	float alpha = 0.55f;		// 不透明度

	float scanPitch = 1.0f;		// 走査線の間隔(モデル:ワールド座標 / 画像:ピクセル)
	float scanSpeed = 4.0f;		// 走査線が流れる速さ(本/秒)
	float scanStrength = 0.7f;	// 走査線の濃さ

	float rimPower = 1.6f;		// 輪郭光の鋭さ(大きいほど細い)
	float rimStrength = 1.0f;	// 輪郭光の強さ
	float brightness = 1.0f;	// 全体の明るさ

	float bandInterval = 60.0f;	// 光の帯の間隔(走査線の本数)
	float bandSpeed = 0.35f;	// 光の帯が流れる速さ(回/秒)
	float bandStrength = 0.35f;	// 光の帯の強さ

	float flicker = 0.08f;		// ちらつきの強さ
	float glitchWidth = 4.0f;	// 横ずれの量(モデル:ワールド座標 / 画像:ピクセル)
	float glitchRate = 0.02f;	// 横ずれが起きる割合

	// モデル向けの初期値
	static HologramParam Model(void) { return HologramParam(); }

	// 画像向けの初期値
	static HologramParam Image(void)
	{
		HologramParam param;
		param.textureRate = 0.35f;
		param.alpha = 0.75f;
		param.scanPitch = 4.0f;
		param.glitchWidth = 12.0f;
		return param;
	}
};
