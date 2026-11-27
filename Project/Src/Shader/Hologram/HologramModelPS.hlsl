#include "../Common/VertexToPixelHeader.hlsli"
#define PS_INPUT VertexToPixelLit
#include "../Common/Pixel/PixelShader3DHeader.hlsli"
#include "HologramCommon.hlsli"

float4 main(PS_INPUT i) : SV_TARGET
{
    float4 tex = diffuseMapTexture.Sample(diffuseMapSampler, i.uv);

    // 抜き部分(髪の毛先など)は深度も書かない
    clip(tex.a - 0.1f);

    // 深度だけを書くパス(色は透明にして画面に影響させない)
    if (g_depthOnly > 0.5f) { return float4(0.0f, 0.0f, 0.0f, 0.0f); }

    // 視線と法線の角度から輪郭を求める
    float3 toCamera = normalize(-i.vwPos);
    float rim = 1.0f - saturate(abs(dot(normalize(i.normal), toCamera)));

    // effectColor.x : ワールド座標の高さ
    float scanPos = i.effectColor.x / g_scanPitch;

    return HologramColor(tex, scanPos, rim);
}
