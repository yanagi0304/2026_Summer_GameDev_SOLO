#include "../PostEffect/PostEffectHeader.hlsli"
#include "HologramCommon.hlsli"

// 画像の端より外は透明として扱う
float4 SampleImage(float2 uv, float2 uvMax)
{
    float2 inside = step(0.0f, uv) * step(uv, uvMax);
    return GetSceneColor(uv) * float4(1.0f, 1.0f, 1.0f, inside.x * inside.y);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    // suv : 画像の右下にあたるUV
    float2 uvMax = input.suv;

    float scanPos = input.pos.y / g_scanPitch;

    // 横ずれ(ピクセル数をUVに変換する)
    float uPerPixel = ddx(input.uv.x);
    float2 uv = input.uv;
    uv.x += HoloGlitch(scanPos) * g_glitchWidth * uPerPixel;

    float4 tex = SampleImage(uv, uvMax);

    // 周囲の透明度との差を輪郭とする
    float2 offset = float2(abs(uPerPixel), abs(ddy(input.uv.y))) * 3.0f;
    float around = SampleImage(uv + float2(offset.x, 0.0f), uvMax).a;
    around = min(around, SampleImage(uv - float2(offset.x, 0.0f), uvMax).a);
    around = min(around, SampleImage(uv + float2(0.0f, offset.y), uvMax).a);
    around = min(around, SampleImage(uv - float2(0.0f, offset.y), uvMax).a);
    float rim = saturate(tex.a - around);

    float4 color = HologramColor(tex, scanPos, rim);
    color.a *= input.diffuse.a;

    return color;
}
