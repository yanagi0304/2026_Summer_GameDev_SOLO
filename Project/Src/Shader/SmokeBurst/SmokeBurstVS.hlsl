// 盾への被弾などで出す「煙がぶわっと弾ける」演出の頂点シェーダー
// 粒(板ポリ)の位置・大きさ・向き・透明度の計算をすべてここで行う
// CPU側は「発生した位置」と「経過時間」を定数バッファで渡すだけ
#include "../Common/Vertex/VertexInputType.hlsli"

// DrawPolygonIndexed3DToShader に渡す VERTEX3DSHADER を入力とする
#define VERTEX_INPUT DX_VERTEX3DSHADER

#include "../Common/Vertex/VertexShader3DHeader.hlsli"

// 1回の発生ごとに変わるパラメータ(b4)
cbuffer SmokeBurstVSParameter : register(b4)
{
    // 煙の発生位置(ワールド座標)
    float3 g_origin;
    // 発生してからの経過時間(秒)
    float g_time;

    // 煙が広がる側の向き(ワールド座標・正規化済み)
    float3 g_hitNormal;
    // 煙が消えるまでの基準時間(秒)
    float g_duration;

    // 粒が飛び出す初速(1秒あたりの移動量)
    float g_burstSpeed;
    // 空気抵抗(大きいほど早く止まる)
    float g_drag;
    // 上へ浮かぶ強さ
    float g_rise;
    // 発生直後の粒の大きさ(半径)
    float g_sizeStart;

    // 消える頃の粒の大きさ(半径)
    float g_sizeEnd;
    // 発生ごとのランダム値(0〜1)。毎回同じ形にならないようにする
    float g_seed;
    float2 g_padding;
};

// ピクセルシェーダーへ渡す値
struct SmokeBurstVSOutput
{
    // 画面上の位置
    float4 svPos : SV_POSITION;
    // 板ポリ内の位置(-1〜1)
    float2 uv : TEXCOORD0;
    // x:年齢(0〜1) y:透明度 z:粒ごとの乱数 w:明るさ
    float4 params : TEXCOORD1;
};

// 法線を軸にした座標系で、ローカル方向をワールド方向へ変換する
float3 LocalToWorld(float3 localDir, float3 n)
{
    // 法線と平行にならない補助軸を選ぶ
    float3 helper = abs(n.y) < 0.99f ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);

    float3 t = normalize(cross(helper, n));
    float3 b = cross(n, t);

    return t * localDir.x + b * localDir.y + n * localDir.z;
}

SmokeBurstVSOutput main(VS_INPUT input)
{
    SmokeBurstVSOutput output = (SmokeBurstVSOutput) 0;

    // 頂点に仕込んである粒ごとの値を取り出す
    // 飛ぶ向き(法線を+Zとしたローカル方向)
    float3 localDir = input.Normal;
    // 初速の倍率
    float speedRate = input.Tangent.x;
    // 大きさの倍率
    float sizeRate = input.Tangent.y;
    // 発生の遅れ(秒)
    float delay = input.Tangent.z;
    // 粒ごとの乱数
    float seedRate = input.Binormal.x;
    // 浮かぶ強さの倍率
    float riseRate = input.Binormal.y;
    // 寿命の倍率
    float lifeRate = input.Binormal.z;
    // 板ポリの角(-1〜1)
    float2 corner = input.TexCoords0;
    // 粒ごとの明るさ
    float brightness = input.diffuse.r;

    // 年齢(0〜1)を求める
    float life = max(g_duration * lifeRate, 0.001f);
    float t = max(g_time - delay, 0.0f);
    float age = saturate(t / life);

    // 発生前・寿命後は大きさ0にして描かない
    float alive = step(delay, g_time) * step(t, life);

    // 発生ごとに飛ぶ向きをひねる(法線まわりに回転)
    float angle = g_seed * 6.2831853f;
    float s;
    float c;
    sincos(angle, s, c);
    float3 twistedDir = float3(
        localDir.x * c - localDir.y * s,
        localDir.x * s + localDir.y * c,
        localDir.z
    );
    float3 dirWorld = LocalToWorld(twistedDir, normalize(g_hitNormal));

    // 空気抵抗で減速しながら飛ぶ距離
    float drag = max(g_drag, 0.001f);
    float dist = g_burstSpeed * speedRate * (1.0f - exp(-drag * t)) / drag;

    // 位置 = 発生位置 + 飛んだ距離 + 上へ浮かぶ分
    float3 worldPos = g_origin
        + dirWorld * dist
        + float3(0.0f, 1.0f, 0.0f) * (g_rise * riseRate * t * t);

    // 大きさ(最初に一気に膨らみ、あとはゆっくり)
    float grow = 1.0f - pow(1.0f - age, 3.0f);
    float size = lerp(g_sizeStart, g_sizeEnd, grow) * sizeRate * alive;

    // 透明度(出だしは素早く現れ、後半でゆっくり消える)
    float fadeIn = smoothstep(0.0f, 0.06f, age);
    float fadeOut = 1.0f - smoothstep(0.35f, 1.0f, age);
    float alpha = fadeIn * fadeOut;

    // カメラ空間で板ポリを広げる(常にカメラを向くビルボード)
    float3 viewPos = mul(float4(worldPos, 1.0f), g_base.viewMatrix);
    viewPos.xy += corner * size;

    output.svPos = mul(float4(viewPos, 1.0f), g_base.projectionMatrix);
    output.uv = corner;
    output.params = float4(age, alpha, frac(seedRate + g_seed), brightness);

    return output;
}
