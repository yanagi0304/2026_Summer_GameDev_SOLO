// ============================================================================
//  PolyDecompose.hlsli
//  ポリゴン分解エフェクト：VS / GS / PS で共有する定義
//
//  DXライブラリの MATRIX は「行ベクトル × 行列」（平行移動が 4 行目）なので
//  row_major で受け取り、mul(float4(p, 1), M) の順で掛ける。
// ============================================================================
#pragma pack_matrix(row_major)

// C++ 側の pd::ConstantData と同じレイアウト（16 バイト境界に注意）
// スロット b4 はユーザー用（b0〜b3 は DXライブラリが内部で使う）
cbuffer PolyDecomposeCB : register(b4)
{
    float4x4 g_ViewProj;      // ビュー × 射影
    float4   g_CamPos;        // xyz : カメラ位置（ワールド）

    float    g_Time;          // 経過時間（秒）
    float    g_Progress;      // 全体の進行度 0〜1
    float    g_FlatMix;       // 0 : スムース法線 / 1 : フラット法線
    float    g_Spread;        // 移動量の倍率

    float    g_Spin;          // 回転量
    float    g_BoundsMinY;    // レスト姿勢での Y の最小値
    float    g_BoundsMaxY;    // レスト姿勢での Y の最大値
    float    g_RMax;          // レスト姿勢での中心からの最大距離

    float4   g_BoundsCenter;  // xyz : レスト姿勢でのバウンディング中心

    float    g_WorldScale;    // モデルの大きさに合わせた移動量のスケール
    int      g_SweepMode;     // 0:上→下 1:下→上 2:中心→外 3:ランダム
    int      g_UseTexture;    // 1 ならテクスチャ t0 を使う / 0 なら UV から模様を生成
    int      g_Pad0;
};

// VS → GS
// 頂点はすでにワールド座標（通常メッシュは生成時点、スケルタルは CPU スキニング後）
struct GS_INPUT
{
    float3 WorldPos : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
    float2 UV       : TEXCOORD2;
    float3 RestPos  : TEXCOORD3;  // レスト姿勢の位置（三角形ごとの乱数と前線の基準）
};

// GS → PS
struct PS_INPUT
{
    float4 Position                 : SV_POSITION;
    float3 WorldPos                 : TEXCOORD0;
    float3 Normal                   : TEXCOORD1;
    float2 UV                       : TEXCOORD2;
    nointerpolation float4 Fx       : TEXCOORD3;  // x: lp  y: 作例2なら1  z: 正方形化の度合い  w: 乱数
};
