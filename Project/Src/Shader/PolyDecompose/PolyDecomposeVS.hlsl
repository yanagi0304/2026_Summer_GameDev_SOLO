// ============================================================================
//  PolyDecomposeVS.hlsl
//  頂点シェーダ：ほぼ素通し。分解の処理はすべて GS で行う。
//
//  入力は DXライブラリの VERTEX3DSHADER（DX_VERTEX_TYPE_SHADER_3D）。
//    pos  : 現在のワールド座標
//    spos : レスト姿勢の位置（xyz）を入れておく
//
//  コンパイル例: fxc /T vs_4_0 /E VSMain /Fo PolyDecomposeVS.vso PolyDecomposeVS.hlsl
// ============================================================================
#include "PolyDecompose.hlsli"

// VERTEX3DSHADER に対応する入力レイアウト
struct VS_INPUT
{
    float3 Position    : POSITION0;
    float4 SubPosition : POSITION1;
    float3 Normal      : NORMAL0;
    float3 Tangent     : TANGENT0;
    float3 Binormal    : BINORMAL0;
    float4 Diffuse     : COLOR0;
    float4 Specular    : COLOR1;
    float2 TexCoords0  : TEXCOORD0;
    float2 TexCoords1  : TEXCOORD1;
};

GS_INPUT main(VS_INPUT input)
{
    GS_INPUT o;
    o.WorldPos = input.Position;
    o.Normal   = input.Normal;
    o.UV       = input.TexCoords0;
    o.RestPos  = input.SubPosition.xyz;
    return o;
}
