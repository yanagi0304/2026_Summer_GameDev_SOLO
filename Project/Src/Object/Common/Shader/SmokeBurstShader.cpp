#include "SmokeBurstShader.h"

#include "../../../pch.h"

#include "../../../Manager/Shader/ShaderResourceManager.h"

#include "../../../Manager/TimeScale/TimeScale.h"

SmokeBurstShader::SmokeBurstShader(const Vector3& origin, const Vector3& normal, float seed) :
	ShaderBase(),
	vertexParam{},
	pixelParam{},
	vertexConstantBuffer(-1),
	pixelConstantBuffer(-1),
	time(0.0f)
{
	// 向きが0なら上向きにして、正規化する
	Vector3 dir = normal;
	if (dir.Length() < 1.0e-4f) { dir = Vector3(0.0f, 1.0f, 0.0f); }
	dir = dir.Normalized();

	// 頂点シェーダー用の値
	// 発生位置
	vertexParam.origin[0] = origin.x;
	vertexParam.origin[1] = origin.y;
	vertexParam.origin[2] = origin.z;
	// 煙が広がる側の向き
	vertexParam.hitNormal[0] = dir.x;
	vertexParam.hitNormal[1] = dir.y;
	vertexParam.hitNormal[2] = dir.z;
	// 消えるまでの基準時間
	vertexParam.duration = DURATION;
	// 粒の初速(1秒あたりの移動量)
	vertexParam.burstSpeed = 300.0f;
	// 空気抵抗(大きいほど早く止まる)
	vertexParam.drag = 4.0f;
	// 上へ浮かぶ強さ
	vertexParam.rise = 120.0f;
	// 発生直後の粒の大きさ
	vertexParam.sizeStart = 20.0f;
	// 消える頃の粒の大きさ
	vertexParam.sizeEnd = 70.0f;
	// 発生ごとの乱数
	vertexParam.seed = seed;

	// ピクセルシェーダー用の値
	// 明るい側の煙の色(ほぼ白)
	pixelParam.lightColor[0] = 1.0f;
	pixelParam.lightColor[1] = 1.0f;
	pixelParam.lightColor[2] = 1.0f;
	pixelParam.lightColor[3] = 1.0f;
	// 影側の煙の色(青みのある灰色)
	pixelParam.shadowColor[0] = 0.55f;
	pixelParam.shadowColor[1] = 0.60f;
	pixelParam.shadowColor[2] = 0.68f;
	pixelParam.shadowColor[3] = 1.0f;
	// 輪郭をノイズで崩す強さ
	pixelParam.edgeNoise = 0.9f;
	// 時間とともに煙が削れていく強さ
	pixelParam.erosion = 0.8f;
}

// シェーダーの初期化
void SmokeBurstShader::Init(void)
{
	// シェーダーハンドル管理クラスを取得
	ShaderResourceManager& manager = ShaderResourceManager::GetIns();

	// シェーダーの生成
	manager.CreateVertexShader(VERTEX_SHADER_TYPE::SmokeBurst);
	manager.CreatePixelShader(PIXEL_SHADER_TYPE::SmokeBurst);

	// 使用するシェーダーを保持
	vertexShaderHandle = manager.GetVertexShader(VERTEX_SHADER_TYPE::SmokeBurst);
	pixelShaderHandle = manager.GetPixelShader(PIXEL_SHADER_TYPE::SmokeBurst);

	// 定数バッファの生成
	vertexConstantBuffer = CreateShaderConstantBuffer(sizeof(VertexParam));
	pixelConstantBuffer = CreateShaderConstantBuffer(sizeof(PixelParam));
}

// シェーダーの更新
void SmokeBurstShader::Update(void)
{
	// 60FPS想定で1フレーム分の時間を進める
	time += (1.0f / 60.0f) * TimeScale::Get();

	vertexParam.time = time;
}

// シェーダーを描画に適用
void SmokeBurstShader::Apply(void)
{
	// 通常のShaderを適用
	ShaderBase::Apply();

	// 頂点シェーダー用の定数バッファ
	if (vertexConstantBuffer >= 0) {
		VertexParam* buffer = static_cast<VertexParam*>(
			GetBufferShaderConstantBuffer(vertexConstantBuffer)
			);

		if (buffer != nullptr) {
			*buffer = vertexParam;

			// CPU側 → GPU側へ反映
			UpdateShaderConstantBuffer(vertexConstantBuffer);
		}

		// SmokeBurstVSのb4に設定
		SetShaderConstantBuffer(vertexConstantBuffer, DX_SHADERTYPE_VERTEX, 4);
	}

	// ピクセルシェーダー用の定数バッファ
	if (pixelConstantBuffer >= 0) {
		PixelParam* buffer = static_cast<PixelParam*>(
			GetBufferShaderConstantBuffer(pixelConstantBuffer)
			);

		if (buffer != nullptr) {
			*buffer = pixelParam;

			// CPU側 → GPU側へ反映
			UpdateShaderConstantBuffer(pixelConstantBuffer);
		}

		// SmokeBurstPSのb4に設定
		SetShaderConstantBuffer(pixelConstantBuffer, DX_SHADERTYPE_PIXEL, 4);
	}
}

// シェーダーの設定リセット
void SmokeBurstShader::ResetApply(void)
{
	// 定数バッファを解除
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_VERTEX, 4);
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_PIXEL, 4);

	// Shader本体を解除
	ShaderBase::ResetApply();
}

// このシェーダーの終了判定
bool SmokeBurstShader::IsEnd(void) const
{
	return time >= TOTAL_TIME;
}

// シェーダー固有リソースの解放
void SmokeBurstShader::Release(void)
{
	// 頂点シェーダー用の定数バッファ
	if (vertexConstantBuffer >= 0) {
		DeleteShaderConstantBuffer(vertexConstantBuffer);
		vertexConstantBuffer = -1;
	}

	// ピクセルシェーダー用の定数バッファ
	if (pixelConstantBuffer >= 0) {
		DeleteShaderConstantBuffer(pixelConstantBuffer);
		pixelConstantBuffer = -1;
	}

	// ShaderResourceManagerから借りているShaderハンドル自体はDeleteしない
	ShaderBase::Release();
}
