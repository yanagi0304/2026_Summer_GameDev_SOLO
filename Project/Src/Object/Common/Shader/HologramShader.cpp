#include "HologramShader.h"

#include "../../../pch.h"

#include "../../../Manager/Shader/ShaderResourceManager.h"

#include "../../../Manager/TimeScale/TimeScale.h"

#include "../Transform/Transform.h"

namespace
{
	// 対応する頂点シェーダーの数
	constexpr int SHADER_NUM = 6;

	// DxLibの頂点タイプに対応する頂点シェーダーの種類を取得(非対応ならfalse)
	// 9フレーム以上の影響を受けるFREE_FRAMEは非対応
	bool ToShaderType(int vertexType, VERTEX_SHADER_TYPE& out)
	{
		switch (vertexType) {
		case DX_MV1_VERTEX_TYPE_1FRAME:      out = VERTEX_SHADER_TYPE::Hologram1Frame;     return true;
		case DX_MV1_VERTEX_TYPE_4FRAME:      out = VERTEX_SHADER_TYPE::Hologram4Frame;     return true;
		case DX_MV1_VERTEX_TYPE_8FRAME:      out = VERTEX_SHADER_TYPE::Hologram8Frame;     return true;
		case DX_MV1_VERTEX_TYPE_NMAP_1FRAME: out = VERTEX_SHADER_TYPE::HologramNMap1Frame; return true;
		case DX_MV1_VERTEX_TYPE_NMAP_4FRAME: out = VERTEX_SHADER_TYPE::HologramNMap4Frame; return true;
		case DX_MV1_VERTEX_TYPE_NMAP_8FRAME: out = VERTEX_SHADER_TYPE::HologramNMap8Frame; return true;
		default: return false;
		}
	}
}

HologramShader::HologramShader(const HologramParam& param) :
	ShaderBase(),
	param(param),
	materialTextureRate(),
	constantBuffer(),
	time(0.0f)
{
}

void HologramShader::Init(void)
{
	// シェーダーハンドル管理クラスを取得
	ShaderResourceManager& manager = ShaderResourceManager::GetIns();

	for (int i = 0; i < SHADER_NUM; i++) {
		manager.CreateVertexShader(static_cast<VERTEX_SHADER_TYPE>(static_cast<int>(VERTEX_SHADER_TYPE::Hologram1Frame) + i));
	}
	manager.CreatePixelShader(PIXEL_SHADER_TYPE::HologramModel);

	pixelShaderHandle = manager.GetPixelShader(PIXEL_SHADER_TYPE::HologramModel);

	constantBuffer.Create();
}

void HologramShader::Update(void)
{
	time += TimeScale::Get() / 60.0f;
}

void HologramShader::Release(void)
{
	constantBuffer.Release();
	ShaderBase::Release();
}

void HologramShader::DrawModel(Transform& trans)
{
	// 動的オブジェクトは毎フレームAttachしなおす
	if (trans.dynamicFlg) { trans.Attach(); }

	Draw(trans.model);
}

void HologramShader::Draw(int model)
{
	if (model == -1) { return; }

	// 現在の描画設定を保存
	int blendMode = DX_BLENDMODE_NOBLEND;
	int blendParam = 0;
	GetDrawBlendMode(&blendMode, &blendParam);

	MV1SetUseOrigShader(true);
	SetUsePixelShader(pixelShaderHandle);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	SetUseZBuffer3D(true);

	// 1回目 : 深度だけを書き込み、裏側や内側の面が透けて重なるのを防ぐ
	SetWriteZBuffer3D(true);
	DrawMeshes(model, true);

	// 2回目 : 一番手前の面だけに色を塗る
	SetWriteZBuffer3D(false);
	DrawMeshes(model, false);
	SetWriteZBuffer3D(true);

	// 描画設定を戻す
	constantBuffer.Reset();
	SetUseVertexShader(-1);
	SetUsePixelShader(-1);
	MV1SetUseOrigShader(false);
	SetDrawBlendMode(blendMode, blendParam);
}

void HologramShader::DrawMeshes(int model, bool depthOnly)
{
	ShaderResourceManager& manager = ShaderResourceManager::GetIns();

	const int meshNum = MV1GetMeshNum(model);
	for (int mesh = 0; mesh < meshNum; mesh++) {

		// 非表示のメッシュは描画しない
		if (MV1GetMeshVisible(model, mesh) == FALSE) { continue; }

		// このメッシュで使う元テクスチャの割合
		const auto itr = materialTextureRate.find(MV1GetMeshMaterial(model, mesh));
		const float textureRate = (itr != materialTextureRate.end()) ? itr->second : param.textureRate;

		constantBuffer.Set(param, time, textureRate, depthOnly);

		const int listNum = MV1GetMeshTListNum(model, mesh);
		for (int i = 0; i < listNum; i++) {
			const int list = MV1GetMeshTList(model, mesh, i);

			// 頂点タイプに合った頂点シェーダーを使う
			VERTEX_SHADER_TYPE shaderType{};
			if (!ToShaderType(MV1GetTriangleListVertexType(model, list), shaderType)) { continue; }
			SetUseVertexShader(manager.GetVertexShader(shaderType));

			MV1DrawTriangleList(model, list);
		}
	}
}
