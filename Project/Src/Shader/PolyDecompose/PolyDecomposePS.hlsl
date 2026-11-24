// ============================================================================
//  PolyDecomposePS.hlsl
//  ポリゴン分解エフェクトのピクセルシェーダー
//  生成演出では、破片はトリオン色(水色)で、元の三角形に戻るにつれてモデル本来の色になる。
// ============================================================================
#include "PolyDecompose.hlsli"

// SetUseTextureToShader(0, ハンドル) で設定したテクスチャ
Texture2D    g_DiffuseTex : register(t0);
SamplerState g_DiffuseSmp : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n  = normalize(input.Normal);
    float3 Vd = normalize(g_CamPos.xyz - input.WorldPos);
    // 両面ライティング(巻き順に依存しないよう視線で判定)
    if (dot(n, Vd) < 0.0) { n = -n; }

    float3 L    = normalize(float3(0.45, 0.85, -0.35));
    float  diff = max(dot(n, L), 0.0);
    float  hemi = lerp(0.40, 0.60, n.y * 0.5 + 0.5);
    float  rim  = pow(1.0 - max(dot(n, Vd), 0.0), 3.0) * 0.25;

    float lp = input.Fx.x;

    float3 albedo = float3(0.8, 0.8, 0.8);
    if (g_UseTexture != 0) { albedo = g_DiffuseTex.Sample(g_DiffuseSmp, input.UV).rgb; }

    float3 col = albedo * (hemi + diff * 0.6);

    // 四角形化の度合い(Fx.z)に応じてトリオン色へ
    const float3 TRION = float3(0.10, 0.75, 0.95);
    col = lerp(col, TRION * (0.5 + 0.5 * diff), input.Fx.z);

    // 分解(または再構成)の最中の破片を光らせる
    float front = step(1e-4, lp) * (1.0 - smoothstep(0.0, 0.5, lp));
    col += float3(0.2, 0.9, 1.0) * front * 1.2;

    // 破片のちらつき
    float flicker = 0.6 + 0.4 * sin(g_Time * 24.0 + input.Fx.w * 50.0);
    col += float3(0.15, 0.8, 1.0) * input.Fx.z * 0.5 * flicker;

    col += rim * float3(0.5, 0.6, 0.8) * input.Fx.z;

    return float4(saturate(col), 1.0);
}
