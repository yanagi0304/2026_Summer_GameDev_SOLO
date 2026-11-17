// 煙が弾ける演出のピクセルシェーダー
// テクスチャは使わず、ノイズで輪郭が崩れた煙の粒を描く

// 見た目の調整用パラメータ(b4)
cbuffer SmokeBurstPSParameter : register(b4)
{
    // 明るい側の煙の色
    float4 g_lightColor;
    // 影側の煙の色
    float4 g_shadowColor;

    // 輪郭をノイズで崩す強さ
    float g_edgeNoise;
    // 時間とともに煙が削れていく強さ
    float g_erosion;
    float2 g_padding;
};

// 頂点シェーダーからの入力
struct SmokeBurstPSInput
{
    // 画面上の位置
    float4 svPos : SV_POSITION;
    // 板ポリ内の位置(-1〜1)
    float2 uv : TEXCOORD0;
    // x:年齢(0〜1) y:透明度 z:粒ごとの乱数 w:明るさ
    float4 params : TEXCOORD1;
};

// 疑似乱数
float Hash(float2 p)
{
    return frac(sin(dot(p, float2(127.1f, 311.7f))) * 43758.5453f);
}

// 2D Value Noise
float Noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);

    // 滑らかな補間
    f = f * f * (3.0f - 2.0f * f);

    float a = Hash(i);
    float b = Hash(i + float2(1.0f, 0.0f));
    float c = Hash(i + float2(0.0f, 1.0f));
    float d = Hash(i + float2(1.0f, 1.0f));

    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

// 3層のノイズを合成(0〜1)
float FBM(float2 p)
{
    float value = 0.0f;
    float amplitude = 0.5f;

    for (int i = 0; i < 3; ++i)
    {
        value += Noise(p) * amplitude;

        p *= 2.0f;
        amplitude *= 0.5f;
    }

    return value / 0.875f;
}

float4 main(SmokeBurstPSInput input) : SV_TARGET
{
    float2 p = input.uv;
    float age = input.params.x;
    float alpha = input.params.y;
    float seed = input.params.z;
    float brightness = input.params.w;

    // 中心からの距離(円の外は描かない)
    float r = length(p);
    clip(1.0f - r);

    // 粒ごとに違うノイズの位置
    float n = FBM(p * 1.8f + seed * 37.0f);

    // 中心ほど濃く、外側をノイズで削る。年齢が進むほどさらに削れる
    float density = (1.0f - r) + (n - 0.5f) * g_edgeNoise - age * g_erosion;
    float a = smoothstep(0.0f, 0.3f, density) * alpha;

    // 上側ほど明るく、ノイズで濃淡をつける
    float light = saturate(0.55f + p.y * 0.25f + (n - 0.5f) * 0.6f);
    float3 color = lerp(g_shadowColor.rgb, g_lightColor.rgb, light) * brightness;

    return float4(color, a);
}
