// ============================================================================
//  PolyDecomposeGS.hlsl
//  ジオメトリシェーダ：三角形ごとにばらばらに動かす本体
//
//  同じファイルからエントリポイントを変えて 2 本コンパイルする。
//    GSShatter : 作例1 飛散     （1 三角形 → 3 頂点）
//    GSQuad    : 作例2 四角形化 （1 三角形 → 4 頂点の TriangleStrip）
//
//  コンパイル例:
//    fxc /T gs_4_0 /E GSShatter /Fo PolyDecomposeGS_Shatter.gso PolyDecomposeGS.hlsl
//    fxc /T gs_4_0 /E GSQuad    /Fo PolyDecomposeGS_Quad.gso    PolyDecomposeGS.hlsl
//
//  WebGPU 版との対応
//    vertex_index / VPT          → GS の入力プリミティブ（三角形）
//    storage から 3 頂点を読む   → triangle GS_INPUT In[3]
//    hash(三角形ID)              → hash(レスト姿勢の重心)
//      ※SV_PrimitiveID は描画呼び出しごとに 0 から振り直されるため、
//        マテリアル単位で分けて描くモデルでは重複する。重心なら重複しない。
// ============================================================================
#include "PolyDecompose.hlsli"

// ---------------------------------------------------------------------------
// 乱数
// ---------------------------------------------------------------------------
uint Pcg(uint v)
{
    uint s = v * 747796405u + 2891336453u;
    uint w = ((s >> ((s >> 28u) + 4u)) ^ s) * 277803737u;
    return (w >> 22u) ^ w;
}

float Hash1(uint n)
{
    return (float)Pcg(n) * (1.0 / 4294967295.0);
}

float3 Hash3(uint n)
{
    return float3(Hash1(n * 3u), Hash1(n * 3u + 1u), Hash1(n * 3u + 2u));
}

// 位置（float3）のビット列から三角形固有のシードを作る
uint SeedOf(float3 p)
{
    uint3 q = asuint(p);
    return Pcg(q.x ^ Pcg(q.y ^ Pcg(q.z)));
}

// ---------------------------------------------------------------------------
// ロドリゲスの回転公式（軸 k は正規化済み）
// ---------------------------------------------------------------------------
float3 RotAxis(float3 v, float3 k, float a)
{
    float c = cos(a);
    float s = sin(a);
    return v * c + cross(k, v) * s + k * dot(k, v) * (1.0 - c);
}

// ---------------------------------------------------------------------------
// 分解の前線：三角形ごとの開始タイミング（0〜1）
// c はレスト姿勢の重心。アニメーションしても前線が揺れない。
// ---------------------------------------------------------------------------
float DelayOf(float3 c, float h)
{
    float range = max(g_BoundsMaxY - g_BoundsMinY, 1e-4);
    float d = h;
    if (g_SweepMode == 0)      d = (g_BoundsMaxY - c.y) / range;
    else if (g_SweepMode == 1) d = (c.y - g_BoundsMinY) / range;
    else if (g_SweepMode == 2) d = length(c - g_BoundsCenter.xyz) / max(g_RMax, 1e-4);
    return saturate(d + (h - 0.5) * 0.1);
}

// ---------------------------------------------------------------------------
// 三角形ごとの共通情報
// ---------------------------------------------------------------------------
struct TriInfo
{
    float3 center;  // 現在の重心
    float3 gnrm;    // 巻き順どおりの幾何法線（正方形の基底に使う）
    float3 fnrm;    // 頂点法線と同じ向きにそろえたフラット法線（ライティング・移動方向）
    float  area2;   // 外積の長さ ＝ 三角形の面積 × 2
    float  h;       // 0〜1 の乱数
    float3 rnd;     // -1〜1 の乱数ベクトル
    float3 axis;    // 回転軸
    float  lp;      // 三角形ごとのローカル進行度
};

static const float FRONT_WIDTH = 0.35;  // 1 枚の三角形が分解しきるまでの幅

TriInfo MakeTri(float3 A, float3 B, float3 C,
                float3 nA, float3 nB, float3 nC,
                float3 rA, float3 rB, float3 rC)
{
    TriInfo t;
    t.center = (A + B + C) / 3.0;

    float3 cr = cross(B - A, C - A);
    t.area2 = length(cr);
    t.gnrm  = cr / max(t.area2, 1e-10);

    // メッシュの巻き順に依存しないよう、頂点法線の向きにそろえる
    t.fnrm = (dot(t.gnrm, nA + nB + nC) < 0.0) ? -t.gnrm : t.gnrm;

    float3 restCenter = (rA + rB + rC) / 3.0;
    uint seed = SeedOf(restCenter);
    t.h    = Hash1(seed);
    t.rnd  = Hash3(seed + 911u) * 2.0 - 1.0;
    t.axis = normalize(t.rnd + float3(1e-3, 2e-3, 3e-3));

    t.lp = saturate((g_Progress * (1.0 + FRONT_WIDTH) - DelayOf(restCenter, t.h)) / FRONT_WIDTH);
    return t;
}

PS_INPUT MakeOut(float3 wpos, float3 nrm, float2 uv, float4 fx)
{
    PS_INPUT o;
    o.Position = mul(float4(wpos, 1.0), g_ViewProj);
    o.WorldPos = wpos;
    o.Normal   = nrm;
    o.UV       = uv;
    o.Fx       = fx;
    return o;
}

// ===========================================================================
// 作例1：飛散
//   法線方向＋ランダム方向へ飛び、落下しながら回転・縮小する。
//   ステートは持たず、すべて lp の関数。
// ===========================================================================
[maxvertexcount(3)]
void GSShatter(triangle GS_INPUT In[3], inout TriangleStream<PS_INPUT> stream)
{
    TriInfo t = MakeTri(In[0].WorldPos, In[1].WorldPos, In[2].WorldPos,
                        In[0].Normal,   In[1].Normal,   In[2].Normal,
                        In[0].RestPos,  In[1].RestPos,  In[2].RestPos);

    float  ang    = t.lp * g_Spin * (0.5 + t.h);
    float  shrink = 1.0 - smoothstep(0.6, 1.0, t.lp);
    float3 vel    = t.fnrm * (0.5 + t.h) + t.rnd * 0.7;
    float3 off    = (vel * t.lp * 1.6 + float3(0.0, -1.4, 0.0) * t.lp * t.lp) * g_Spread * g_WorldScale;
    float4 fx     = float4(t.lp, 0.0, 0.0, t.h);

    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float3 local = RotAxis(In[i].WorldPos - t.center, t.axis, ang) * shrink;
        float3 n = normalize(lerp(In[i].Normal, t.fnrm, g_FlatMix));
        n = RotAxis(normalize(lerp(n, t.fnrm, t.lp)), t.axis, ang);
        stream.Append(MakeOut(t.center + off + local, n, In[i].UV, fx));
    }
    stream.RestartStrip();
}

// ===========================================================================
// 作例2：四角形化
//   角 q0..q3 を TriangleStrip（q0, q1, q3, q2）で出力する。
//   q3 は最初 q2（元三角形の 3 つ目の角）に重ねておき、
//   lp に応じて同じ面積の正方形の 4 隅へ補間する。
//   回転・上昇・縮小は 6 段階に量子化して、デジタルな動きにする。
// ===========================================================================
[maxvertexcount(4)]
void main(triangle GS_INPUT In[3], inout TriangleStream<PS_INPUT> stream)
{
    TriInfo t = MakeTri(In[0].WorldPos, In[1].WorldPos, In[2].WorldPos,
                        In[0].Normal,   In[1].Normal,   In[2].Normal,
                        In[0].RestPos,  In[1].RestPos,  In[2].RestPos);

    // 正方形の基底。巻き順と同じ向きにするため幾何法線 gnrm を使う
    float3 T  = normalize(In[1].WorldPos - In[0].WorldPos);
    float3 Bt = cross(t.gnrm, T);
    float  hs = sqrt(t.area2 * 0.5) * 0.5;  // 同じ面積の正方形の半辺

    float m     = smoothstep(0.0, 0.35, t.lp);
    float st    = smoothstep(0.3, 1.0, t.lp);
    float steps = floor(st * 6.0) / 6.0;
    float dir   = (t.h > 0.5) ? 1.0 : -1.0;
    float a1    = steps * g_Spin * 0.6 * dir;
    float a2    = steps * 1.2;
    float scale = 1.0 - steps * steps;

    float3 off = (float3(0.0, steps * 1.4, 0.0) + t.fnrm * steps * 0.25) * g_Spread * g_WorldScale;
    float4 fx  = float4(t.lp, 1.0, m, t.h);

    const float2 SQ[4]    = { float2(-1.0, -1.0), float2(1.0, -1.0), float2(1.0, 1.0), float2(-1.0, 1.0) };
    const uint   ORDER[4] = { 0u, 1u, 3u, 2u };  // Strip の出力順

    for (int k = 0; k < 4; ++k)
    {
        uint q = ORDER[k];
        uint c = min(q, 2u);  // q3 は元三角形の 3 つ目の角から始まる

        float3 sqLocal = (T * SQ[q].x + Bt * SQ[q].y) * hs;
        float3 local   = lerp(In[c].WorldPos - t.center, sqLocal, m);
        local = RotAxis(RotAxis(local, t.fnrm, a1), t.axis, a2) * scale;

        float3 n = normalize(lerp(In[c].Normal, t.fnrm, g_FlatMix));
        n = RotAxis(RotAxis(normalize(lerp(n, t.fnrm, m)), t.fnrm, a1), t.axis, a2);

        stream.Append(MakeOut(t.center + off + local, n, In[c].UV, fx));
    }
    stream.RestartStrip();
}
