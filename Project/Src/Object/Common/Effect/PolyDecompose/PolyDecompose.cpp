#include "PolyDecompose.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "../../../../Manager/Shader/ShaderResourceManager.h"
#include "../../../../Manager/TimeScale/TimeScale.h"

int PolyDecompose::sharedConstantBuffer = -1;
int PolyDecompose::sharedRefCount = 0;

PolyDecompose::PolyDecompose(void) :
	model(-1),
	vertexBuffer(-1),
	vertices(),
	batches(),
	sharedAcquired(false),
	restCaptured(false),
	boundsMinY(0.0f),
	boundsMaxY(1.0f),
	boundsRMax(1.0f),
	boundsCenter(VGet(0.0f, 0.0f, 0.0f)),
	time(0.0f),
	data{}
{
}

bool PolyDecompose::Init(int model)
{
	if (model < 0) { return false; }

	// 非同期読み込み中・読み込み失敗のモデルは使えない(頂点数が不正な値になる)
	if (CheckHandleASyncLoad(model) != FALSE) { return false; }

	this->model = model;

	// 参照用メッシュ: 全フレーム、ワールド座標で取得する
	MV1SetupReferenceMesh(model, -1, TRUE);
	MV1RefreshReferenceMesh(model, -1, TRUE);
	const MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(model, -1, TRUE);
	if (ref.VertexNum <= 0 || ref.PolygonNum <= 0 || ref.Vertexs == nullptr || ref.Polygons == nullptr) { return false; }

	// 頂点バッファ
	vertices.assign(ref.VertexNum, VERTEX3DSHADER{});
	vertexBuffer = CreateVertexBuffer(ref.VertexNum, DX_VERTEX_TYPE_SHADER_3D);
	if (vertexBuffer < 0) { return false; }
	CaptureVertices(true);

	// マテリアルごとにインデックスを分ける(テクスチャを切り替えるため)
	const int materialNum = MV1GetMaterialNum(model);
	std::vector<std::vector<unsigned int>> byMaterial((std::max)(materialNum, 1));
	for (int p = 0; p < ref.PolygonNum; ++p)
	{
		const MV1_REF_POLYGON& poly = ref.Polygons[p];
		const int m = (poly.MaterialIndex >= 0 && poly.MaterialIndex < materialNum) ? poly.MaterialIndex : 0;
		byMaterial[m].push_back(static_cast<unsigned int>(poly.VIndex[0]));
		byMaterial[m].push_back(static_cast<unsigned int>(poly.VIndex[1]));
		byMaterial[m].push_back(static_cast<unsigned int>(poly.VIndex[2]));
	}

	for (int m = 0; m < static_cast<int>(byMaterial.size()); ++m)
	{
		if (byMaterial[m].empty()) { continue; }

		Batch batch;
		batch.indexBuffer = CreateIndexBuffer(static_cast<int>(byMaterial[m].size()), DX_INDEX_TYPE_32BIT);
		if (batch.indexBuffer < 0) { continue; }
		SetIndexBufferData(0, byMaterial[m].data(), static_cast<int>(byMaterial[m].size()), batch.indexBuffer);
		batch.triangleCount = static_cast<int>(byMaterial[m].size() / 3);

		if (m < materialNum)
		{
			const int texIndex = MV1GetMaterialDifMapTexture(model, m);
			if (texIndex >= 0) { batch.texture = MV1GetTextureGraphHandle(model, texIndex); }
		}
		batches.push_back(batch);
	}

	// シェーダー(ハンドルはShaderResourceManagerが持つ)
	ShaderResourceManager& manager = ShaderResourceManager::GetIns();
	manager.CreateVertexShader(VERTEX_SHADER_TYPE::PolyDecompose);
	manager.CreateGeometryShader(GEOMETRY_SHADER_TYPE::PolyDecompose);
	manager.CreatePixelShader(PIXEL_SHADER_TYPE::PolyDecompose);

	// 定数バッファは全員で共有する
	if (sharedRefCount == 0) { sharedConstantBuffer = CreateShaderConstantBuffer(sizeof(ConstantData)); }
	++sharedRefCount;
	sharedAcquired = true;

	// 最初の姿勢は、Updateの最初の1回で取り込み直す(アニメーションが始まった後の姿勢にするため)
	restCaptured = false;
	time = 0.0f;

	return true;
}

void PolyDecompose::CaptureVertices(bool rest)
{
	MV1RefreshReferenceMesh(model, -1, TRUE);
	const MV1_REF_POLYGONLIST ref = MV1GetReferenceMesh(model, -1, TRUE);

	const int num = (std::min)(ref.VertexNum, static_cast<int>(vertices.size()));
	for (int i = 0; i < num; ++i)
	{
		const MV1_REF_VERTEX& src = ref.Vertexs[i];
		VERTEX3DSHADER& dst = vertices[i];

		dst.pos = src.Position;
		dst.norm = src.Normal;

		if (rest)
		{
			dst.spos.x = src.Position.x;
			dst.spos.y = src.Position.y;
			dst.spos.z = src.Position.z;
			dst.spos.w = 1.0f;

			dst.tan = VGet(0.0f, 0.0f, 0.0f);
			dst.binorm = VGet(0.0f, 0.0f, 0.0f);
			dst.dif = GetColorU8(255, 255, 255, 255);
			dst.spc = GetColorU8(0, 0, 0, 0);
			dst.u = src.TexCoord[0].u;
			dst.v = src.TexCoord[0].v;
			dst.su = dst.u;
			dst.sv = dst.v;
		}
	}

	SetVertexBufferData(0, vertices.data(), num, vertexBuffer);
}

void PolyDecompose::ComputeBounds(void)
{
	VECTOR mn = VGet(vertices[0].spos.x, vertices[0].spos.y, vertices[0].spos.z);
	VECTOR mx = mn;
	for (const VERTEX3DSHADER& v : vertices)
	{
		mn.x = (std::min)(mn.x, v.spos.x); mx.x = (std::max)(mx.x, v.spos.x);
		mn.y = (std::min)(mn.y, v.spos.y); mx.y = (std::max)(mx.y, v.spos.y);
		mn.z = (std::min)(mn.z, v.spos.z); mx.z = (std::max)(mx.z, v.spos.z);
	}

	boundsMinY = mn.y;
	boundsMaxY = mx.y;
	boundsCenter = VScale(VAdd(mn, mx), 0.5f);

	float r2 = 0.0f;
	for (const VERTEX3DSHADER& v : vertices)
	{
		const float dx = v.spos.x - boundsCenter.x;
		const float dy = v.spos.y - boundsCenter.y;
		const float dz = v.spos.z - boundsCenter.z;
		r2 = (std::max)(r2, dx * dx + dy * dy + dz * dz);
	}
	boundsRMax = (std::max)(std::sqrt(r2), 1.0e-4f);
}

void PolyDecompose::Update(void)
{
	if (!IsValid()) { return; }

	if (!restCaptured)
	{
		// 最初の1回だけ、sposも含めて取り込み直して、分解の前線と乱数の基準にする
		CaptureVertices(true);
		ComputeBounds();
		restCaptured = true;
	}
	else
	{
		CaptureVertices(false);
	}

	time += (1.0f / 60.0f) * TimeScale::Get();
}

void PolyDecompose::Draw(float progress, const Params& params)
{
	if (!IsValid() || sharedConstantBuffer < 0 || !restCaptured) { return; }

	ShaderResourceManager& manager = ShaderResourceManager::GetIns();

	// 定数バッファの共通部分
	const MATRIX viewProj = MMult(GetCameraViewMatrix(), GetCameraProjectionMatrix());
	std::memcpy(data.viewProj, &viewProj, sizeof(data.viewProj));

	const VECTOR cam = GetCameraPosition();
	data.camPos[0] = cam.x;
	data.camPos[1] = cam.y;
	data.camPos[2] = cam.z;
	data.camPos[3] = 1.0f;

	data.time = time;
	data.progress = std::clamp(progress, 0.0f, 1.0f);
	data.flatMix = params.flatMix;
	data.spread = params.spread;
	data.spin = params.spin;
	data.sweepMode = params.sweepMode;

	data.boundsMinY = boundsMinY;
	data.boundsMaxY = boundsMaxY;
	data.rMax = boundsRMax;
	data.boundsCenter[0] = boundsCenter.x;
	data.boundsCenter[1] = boundsCenter.y;
	data.boundsCenter[2] = boundsCenter.z;
	data.boundsCenter[3] = 0.0f;
	data.worldScale = boundsRMax / REFERENCE_RADIUS;

	// 分解中は裏面も見えるので両面描画にする(終わったら元に戻す)
	const int prevCulling = GetUseBackCulling();
	SetUseBackCulling(DX_CULLING_NONE);

	SetUseVertexShader(manager.GetVertexShader(VERTEX_SHADER_TYPE::PolyDecompose));
	SetUseGeometryShader(manager.GetGeometryShader(GEOMETRY_SHADER_TYPE::PolyDecompose));
	SetUsePixelShader(manager.GetPixelShader(PIXEL_SHADER_TYPE::PolyDecompose));

	for (const Batch& batch : batches)
	{
		data.useTexture = (batch.texture >= 0) ? 1 : 0;

		// CPU側 → GPU側へ反映して、GSとPSのb4に設定する
		void* dst = GetBufferShaderConstantBuffer(sharedConstantBuffer);
		if (dst != nullptr) { std::memcpy(dst, &data, sizeof(data)); }
		UpdateShaderConstantBuffer(sharedConstantBuffer);
		SetShaderConstantBuffer(sharedConstantBuffer, DX_SHADERTYPE_GEOMETRY, 4);
		SetShaderConstantBuffer(sharedConstantBuffer, DX_SHADERTYPE_PIXEL, 4);

		SetUseTextureToShader(0, batch.texture);
		DrawPolygonIndexed3DToShader_UseVertexBuffer(vertexBuffer, batch.indexBuffer);
	}

	// 設定を元に戻す
	SetUseTextureToShader(0, -1);
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_GEOMETRY, 4);
	SetShaderConstantBuffer(-1, DX_SHADERTYPE_PIXEL, 4);
	SetUseGeometryShader(-1);
	SetUseVertexShader(-1);
	SetUsePixelShader(-1);
	SetUseBackCulling(prevCulling);
}

void PolyDecompose::Release(void)
{
	for (Batch& batch : batches)
	{
		if (batch.indexBuffer >= 0) { DeleteIndexBuffer(batch.indexBuffer); }
	}
	batches.clear();

	if (vertexBuffer >= 0)
	{
		DeleteVertexBuffer(vertexBuffer);
		vertexBuffer = -1;
	}
	vertices.clear();

	// 共有の定数バッファは、最後の1つが解放される時に消す
	if (sharedAcquired)
	{
		sharedAcquired = false;
		--sharedRefCount;
		if (sharedRefCount == 0 && sharedConstantBuffer >= 0)
		{
			DeleteShaderConstantBuffer(sharedConstantBuffer);
			sharedConstantBuffer = -1;
		}
	}

	// モデルは持ち主が解放する
	model = -1;
}
