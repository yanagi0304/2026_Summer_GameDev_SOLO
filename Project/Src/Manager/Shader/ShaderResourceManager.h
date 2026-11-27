#pragma once

#include <map>
#include <string>

#include "ShaderTypeDefine.h"

class ShaderResourceManager
{
private:

	static ShaderResourceManager* ins;

	ShaderResourceManager() :
		vertexShaderList(),
		pixelShaderList(),
			geometryShaderList()
	{
	}
	~ShaderResourceManager() = default;

public:

	static void CreateIns(void) { if (ins == nullptr)ins = new ShaderResourceManager(); }
	static ShaderResourceManager& GetIns(void) { return *ins; }
	static void DeleteIns(void) {
		if (ins != nullptr) { 
			ins->ReleaseAllShader();
			delete ins; 
			ins = nullptr;
		}
	}


	// 頂点シェーダーの生成
	void CreateVertexShader(VERTEX_SHADER_TYPE type);

	// 頂点シェーダーの取得
	int GetVertexShader(VERTEX_SHADER_TYPE type);

	// 頂点シェーダーの解放
	void ReleaseVertexShader(VERTEX_SHADER_TYPE type);

	// 頂点シェーダーを全て解放する
	void ReleaseAllVertexShader(void);


	// ピクセルシェーダーの生成
	void CreatePixelShader(PIXEL_SHADER_TYPE type);

	// ピクセルシェーダーの取得
	int GetPixelShader(PIXEL_SHADER_TYPE type);

	// ピクセルシェーダーの解放
	void ReleasePixelShader(PIXEL_SHADER_TYPE type);

	// ピクセルシェーダーを全て解放する
	void ReleaseAllPixelShader(void);


	// ジオメトリシェーダーの生成
	void CreateGeometryShader(GEOMETRY_SHADER_TYPE type);

	// ジオメトリシェーダーの取得
	int GetGeometryShader(GEOMETRY_SHADER_TYPE type);

	// ジオメトリシェーダーの解放
	void ReleaseGeometryShader(GEOMETRY_SHADER_TYPE type);

	// ジオメトリシェーダーを全て解放する
	void ReleaseAllGeometryShader(void);


	// 全てのシェーダーの解放
	void ReleaseAllShader(void);

private:

	// シェーダーの名前をパスへ
	std::string ShaderNameToPath(const std::string& shaderName) {
		return "Data/Shader/" + shaderName + ".cso";
	}

	// 頂点シェーダーの名前テーブル
	const std::map<VERTEX_SHADER_TYPE, std::string> VERTEX_SHADER_NAME_TABLE = {

	#pragma region デフォルト
		{
			VERTEX_SHADER_TYPE::Default1Frame,
			"DefaultVS_1Frame"
		},
		{
			VERTEX_SHADER_TYPE::Default4Frame,
			"DefaultVS_4Frame"
		},
		{
			VERTEX_SHADER_TYPE::Default8Frame,
			"DefaultVS_8Frame"
		},
		{
			VERTEX_SHADER_TYPE::DefaultNMap1Frame,
			"DefaultVS_NMap1Frame"
		},
		{
			VERTEX_SHADER_TYPE::DefaultNMap4Frame,
			"DefaultVS_NMap4Frame"
		},
		{
			VERTEX_SHADER_TYPE::DefaultNMap8Frame,
			"DefaultVS_NMap8Frame"
		},
	#pragma endregion

	#pragma region リムライト
		{
			VERTEX_SHADER_TYPE::RimLight1Frame,
			"RimLightVS_1Frame"
		},
		{
			VERTEX_SHADER_TYPE::RimLight4Frame,
			"RimLightVS_4Frame"
		},
		{
			VERTEX_SHADER_TYPE::RimLight8Frame,
			"RimLightVS_8Frame"
		},
		{
			VERTEX_SHADER_TYPE::RimLightNMap1Frame,
			"RimLightVS_NMap1Frame"
		},
		{
			VERTEX_SHADER_TYPE::RimLightNMap4Frame,
			"RimLightVS_NMap4Frame"
		},
		{
			VERTEX_SHADER_TYPE::RimLightNMap8Frame,
			"RimLightVS_NMap8Frame"
		},
	#pragma endregion

		// 水面
		{
			VERTEX_SHADER_TYPE::Water,
			"WaterVS"
		},

		// スカイボックス
		{
			VERTEX_SHADER_TYPE::SkyBox,
			"SkyBoxVS"
		},

		// 煙エフェクト
		{
			VERTEX_SHADER_TYPE::SmokeBurst,
			"SmokeBurstVS"
		},

		// ポリゴン分解
		{
			VERTEX_SHADER_TYPE::PolyDecompose,
			"PolyDecomposeVS"
		},

		// ホログラム
		{
			VERTEX_SHADER_TYPE::Hologram1Frame,
			"HologramVS_1Frame"
		},

		{
			VERTEX_SHADER_TYPE::Hologram4Frame,
			"HologramVS_4Frame"
		},

		{
			VERTEX_SHADER_TYPE::Hologram8Frame,
			"HologramVS_8Frame"
		},

		{
			VERTEX_SHADER_TYPE::HologramNMap1Frame,
			"HologramVS_NMap1Frame"
		},

		{
			VERTEX_SHADER_TYPE::HologramNMap4Frame,
			"HologramVS_NMap4Frame"
		},

		{
			VERTEX_SHADER_TYPE::HologramNMap8Frame,
			"HologramVS_NMap8Frame"
		},
	};

	// 頂点シェーダー
	std::map<VERTEX_SHADER_TYPE, int> vertexShaderList;


	// ピクセルシェーダーの名前テーブル
	const std::map<PIXEL_SHADER_TYPE, std::string> PIXEL_SHADER_NAME_TABLE = {

		// デフォルト
		{
			PIXEL_SHADER_TYPE::Default,
			"DefaultPS"
		},

		// リムライト
		{
			PIXEL_SHADER_TYPE::RimLight,
			"RimLightPS"
		},

		// 水面
		{
			PIXEL_SHADER_TYPE::Water,
			"WaterPS"
		},

	#pragma region ポストエフェクト

		// ブラウン管
		{
			PIXEL_SHADER_TYPE::CRT,
			"CRT_PS"
		},

		// 集中線
		{
			PIXEL_SHADER_TYPE::FocusLines,
			"FocusLinesPS"
		},

		// 集中線
		{
			PIXEL_SHADER_TYPE::UnderWater,
			"UnderWaterPS"
		},

		// ゴゴゴゴ…エフェクト
		{
			PIXEL_SHADER_TYPE::Menacing,
			"MenacingPS"
		},
	#pragma endregion

		// スカイボックス
		{
			PIXEL_SHADER_TYPE::SkyBox,
			"SkyBoxPS"
		},

		#pragma region エフェクト

		// 煙エフェクト
		{
			PIXEL_SHADER_TYPE::SmokeBurst,
			"SmokeBurstPS"
		},

		// ポリゴン分解
		{
			PIXEL_SHADER_TYPE::PolyDecompose,
			"PolyDecomposePS"
		},

		// ホログラム(モデル)
		{
			PIXEL_SHADER_TYPE::HologramModel,
			"HologramModelPS"
		},

		// ホログラム(画像)
		{
			PIXEL_SHADER_TYPE::HologramImage,
			"HologramImagePS"
		},

		#pragma endregion

	};

	// ピクセルシェーダー
	std::map<PIXEL_SHADER_TYPE, int> pixelShaderList;


	// ジオメトリシェーダーの名前テーブル
	const std::map<GEOMETRY_SHADER_TYPE, std::string> GEOMETRY_SHADER_NAME_TABLE = {

		// ポリゴン分解
		{
			GEOMETRY_SHADER_TYPE::PolyDecompose,
			"PolyDecomposeGS"
		},
	};

	// ジオメトリシェーダー
	std::map<GEOMETRY_SHADER_TYPE, int> geometryShaderList;
};