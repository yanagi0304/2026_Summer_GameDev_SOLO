#ifndef HOLOGRAM_COMMON_HLSLI
#define HOLOGRAM_COMMON_HLSLI

// C++側の HologramConstant と並びを一致させること
cbuffer HologramParam : register(b4)
{
    float3 g_holoColor;     // ホログラムの色
    float g_time;           // 経過時間(秒)

    float g_textureRate;    // 元テクスチャの色を残す割合
    float g_alpha;          // 不透明度
    float g_scanPitch;      // 走査線の間隔(モデル:ワールド座標 / 画像:ピクセル)
    float g_scanSpeed;      // 走査線が流れる速さ(本/秒)

    float g_scanStrength;   // 走査線の濃さ
    float g_rimPower;       // 輪郭光の鋭さ
    float g_rimStrength;    // 輪郭光の強さ
    float g_brightness;     // 全体の明るさ

    float g_bandInterval;   // 光の帯の間隔(走査線の本数)
    float g_bandSpeed;      // 光の帯が流れる速さ(回/秒)
    float g_bandStrength;   // 光の帯の強さ
    float g_flicker;        // ちらつきの強さ

    float g_glitchWidth;    // 横ずれの量(モデル:ワールド座標 / 画像:ピクセル)
    float g_glitchRate;     // 横ずれが起きる割合
    float g_depthOnly;      // 1なら深度の書き込みのみ
    float g_holoPadding;
};

float HoloHash(float2 v)
{
    return frac(sin(dot(v, float2(12.9898f, 78.233f))) * 43758.5453f);
}

// 高さ scanPos の帯が今ずれる量(-0.5 ～ 0.5)
float HoloGlitch(float scanPos)
{
    float slice = floor(scanPos / 8.0f);
    float tick = floor(g_time * 12.0f);
    float hit = step(1.0f - g_glitchRate, HoloHash(float2(slice, tick)));
    return (HoloHash(float2(tick, slice)) - 0.5f) * hit;
}

// tex     : 元の色
// scanPos : 縦方向の位置(走査線1本で1増える)
// rim     : 輪郭の度合い(0 ～ 1)
float4 HologramColor(float4 tex, float scanPos, float rim)
{
    // 走査線
    float phase = scanPos - g_time * g_scanSpeed;
    float stripe = smoothstep(0.2f, 0.8f, abs(frac(phase) - 0.5f) * 2.0f);
    // 線が細かくなりすぎる距離では平均値に寄せてモアレを防ぐ
    stripe = lerp(stripe, 0.5f, saturate(fwidth(phase) * 2.5f - 0.5f));
    float scan = lerp(1.0f, stripe, g_scanStrength);

    // ゆっくり流れる光の帯
    float bandPhase = frac(scanPos / max(g_bandInterval, 1.0f) - g_time * g_bandSpeed);
    float band = pow(bandPhase, 12.0f) * g_bandStrength;

    // ちらつき
    float flicker = 1.0f - g_flicker * HoloHash(float2(floor(g_time * 24.0f), 0.5f));

    // 明るさを単色に置き換えた色と元の色を混ぜる
    float lum = dot(tex.rgb, float3(0.299f, 0.587f, 0.114f));
    float3 mono = g_holoColor * (0.4f + lum * 1.0f);
    float3 color = lerp(mono, tex.rgb, g_textureRate);

    float rimLight = pow(saturate(rim), g_rimPower) * g_rimStrength;

    color *= lerp(1.0f, scan * 1.5f, g_scanStrength);
    color += g_holoColor * (rimLight + band);
    color *= g_brightness * flicker;

    // テクスチャを残すほど透けにくくする
    float alpha = lerp(g_alpha, saturate(g_alpha + 0.3f), g_textureRate);
    alpha *= lerp(1.0f, 0.35f + 0.65f * stripe, g_scanStrength);
    alpha = saturate(alpha + rimLight * 0.5f + band * 0.5f);

    return float4(color, alpha * tex.a * flicker);
}

#endif
