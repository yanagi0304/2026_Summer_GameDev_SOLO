#include "../Common/VertexToPixelHeader.hlsli"
#define VS_OUTPUT VertexToPixelLit
#include "../Common/Vertex/VertexShader3DHeader.hlsli"
#include "../Common/Vertex/ModelVertexTransform.hlsli"
#include "HologramCommon.hlsli"

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o = (VS_OUTPUT)0;
    float3 wp = ModelToWorldPosition(input), wn = ModelToWorldNormal(input);
    float3 vp = WorldToViewPosition(wp), vn = WorldToViewNormal(wn);

    // ‰¡‚¸‚ê(‰æ–Ê‚Ì‰¡•ûŒü‚É‚¸‚ç‚·)
    vp.x += HoloGlitch(wp.y / g_scanPitch) * g_glitchWidth;

    o.svPos = ViewToProjection(vp);
    o.vwPos = vp;
    o.normal = vn;
    o.uv = input.uv0.xy;
    o.diffuse = input.diffuse;
    o.effectColor = float4(wp.y, 0.0f, 0.0f, 0.0f);
    return o;
}
