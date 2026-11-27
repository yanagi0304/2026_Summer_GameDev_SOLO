#include "HologramImageShader.h"

#include <cmath>

#include "../../../pch.h"

#include "../../../Manager/Shader/ShaderResourceManager.h"

#include "../../../Manager/TimeScale/TimeScale.h"

HologramImageShader::HologramImageShader(const HologramParam& param) :
	param(param),
	constantBuffer(),
	pixelShaderHandle(-1),
	time(0.0f)
{
}

void HologramImageShader::Init(void)
{
	// シェーダーハンドル管理クラスを取得
	ShaderResourceManager& manager = ShaderResourceManager::GetIns();

	manager.CreatePixelShader(PIXEL_SHADER_TYPE::HologramImage);
	pixelShaderHandle = manager.GetPixelShader(PIXEL_SHADER_TYPE::HologramImage);

	constantBuffer.Create();
}

void HologramImageShader::Update(void)
{
	time += TimeScale::Get() / 60.0f;
}

void HologramImageShader::Release(void)
{
	constantBuffer.Release();
	pixelShaderHandle = -1;
}

void HologramImageShader::Draw(int x, int y, int width, int height, int graphHandle)
{
	DrawQuad(
		x + width * 0.5f, y + height * 0.5f,
		static_cast<float>(width), static_cast<float>(height),
		0.0f, graphHandle
	);
}

void HologramImageShader::DrawRota(int x, int y, float scale, float angle, int graphHandle)
{
	int width = 0;
	int height = 0;
	if (GetGraphSize(graphHandle, &width, &height) == -1) { return; }

	DrawQuad(
		static_cast<float>(x), static_cast<float>(y),
		width * scale, height * scale,
		angle, graphHandle
	);
}

void HologramImageShader::DrawQuad(float centerX, float centerY, float width, float height, float angle, int graphHandle)
{
	// 画像サイズとテクスチャサイズからUVの右下を求める
	int graphWidth = 0;
	int graphHeight = 0;
	if (GetGraphSize(graphHandle, &graphWidth, &graphHeight) == -1) { return; }

	int textureWidth = 0;
	int textureHeight = 0;
	if (GetGraphTextureSize(graphHandle, &textureWidth, &textureHeight) == -1) { return; }

	const float uMax = static_cast<float>(graphWidth) / textureWidth;
	const float vMax = static_cast<float>(graphHeight) / textureHeight;

	const float cosA = std::cos(angle);
	const float sinA = std::sin(angle);

	// 左上、右上、左下、右下
	VERTEX2DSHADER vertex[4]{};
	for (int i = 0; i < 4; i++) {
		const bool isRight = (i % 2) == 1;
		const bool isBottom = i >= 2;

		const float localX = (isRight ? 0.5f : -0.5f) * width;
		const float localY = (isBottom ? 0.5f : -0.5f) * height;

		vertex[i].pos.x = centerX + localX * cosA - localY * sinA;
		vertex[i].pos.y = centerY + localX * sinA + localY * cosA;
		vertex[i].pos.z = 0.0f;
		vertex[i].rhw = 1.0f;
		vertex[i].dif = GetColorU8(255, 255, 255, 255);
		vertex[i].u = isRight ? uMax : 0.0f;
		vertex[i].v = isBottom ? vMax : 0.0f;
		vertex[i].su = uMax;
		vertex[i].sv = vMax;
	}

	unsigned short index[6]{ 0, 1, 2, 1, 3, 2 };

	// 現在の描画設定を保存
	int blendMode = DX_BLENDMODE_NOBLEND;
	int blendParam = 0;
	GetDrawBlendMode(&blendMode, &blendParam);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	SetUseTextureToShader(0, graphHandle);
	SetUsePixelShader(pixelShaderHandle);
	constantBuffer.Set(param, time, param.textureRate, false);

	DrawPolygonIndexed2DToShader(vertex, 4, index, 2);

	// 描画設定を戻す
	constantBuffer.Reset();
	SetUsePixelShader(-1);
	SetUseTextureToShader(0, -1);
	SetDrawBlendMode(blendMode, blendParam);
}
