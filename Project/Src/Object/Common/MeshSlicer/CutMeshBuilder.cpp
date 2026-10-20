#include "CutMeshBuilder.h"

#include <map>

#include "../../../pch.h"

namespace
{
	// 0～255 を 0.0～1.0 に変換する
	float ToFloat(unsigned char value)
	{
		return static_cast<float>(value) / 255.0f;
	}
}

bool CutMeshBuilder::FromModel(int modelHandle, CutMesh& outMesh, int frameIndex)
{
	outMesh.Clear();

	if (modelHandle < 0) { return false; }

	// 参照用メッシュはモデルのワールド行列を反映した座標で返ってくる
	// いったん単位行列にして、モデル自身のローカル座標で取得する
	// （アクター側のTransformで配置するため、元の行列は取得後に戻す）
	const MATRIX originalMat = MV1GetMatrix(modelHandle);
	MV1SetMatrix(modelHandle, MGetIdent());

	const MV1_REF_POLYGONLIST referenceMesh =
		MV1GetReferenceMesh(modelHandle, frameIndex, TRUE, FALSE);

	MV1SetMatrix(modelHandle, originalMat);

	if (referenceMesh.PolygonNum <= 0 ||
		referenceMesh.VertexNum <= 0 ||
		referenceMesh.Polygons == nullptr ||
		referenceMesh.Vertexs == nullptr)
	{
		return false;
	}

	// 頂点
	outMesh.vertices.resize(referenceMesh.VertexNum);

	for (int i = 0; i < referenceMesh.VertexNum; i++) {

		const MV1_REF_VERTEX& src = referenceMesh.Vertexs[i];
		CutVertex& dst = outMesh.vertices[i];

		dst.pos = Vector3(src.Position);
		dst.normal = Vector3(src.Normal);

		dst.u = src.TexCoord[0].u;
		dst.v = src.TexCoord[0].v;

		dst.color.r = ToFloat(src.DiffuseColor.r);
		dst.color.g = ToFloat(src.DiffuseColor.g);
		dst.color.b = ToFloat(src.DiffuseColor.b);
		dst.color.a = ToFloat(src.DiffuseColor.a);
	}

	// 三角形（マテリアル番号ごとのサブメッシュへ振り分ける）
	std::map<int, size_t> subMeshOf;

	for (int i = 0; i < referenceMesh.PolygonNum; i++) {

		const MV1_REF_POLYGON& polygon = referenceMesh.Polygons[i];

		const int i0 = polygon.VIndex[0];
		const int i1 = polygon.VIndex[1];
		const int i2 = polygon.VIndex[2];

		// 範囲外・つぶれた三角形は捨てる
		if (i0 < 0 || i1 < 0 || i2 < 0) { continue; }
		if (i0 >= referenceMesh.VertexNum || i1 >= referenceMesh.VertexNum || i2 >= referenceMesh.VertexNum) { continue; }
		if (i0 == i1 || i1 == i2 || i2 == i0) { continue; }

		const int materialIndex = static_cast<int>(polygon.MaterialIndex);

		auto it = subMeshOf.find(materialIndex);

		if (it == subMeshOf.end()) {

			outMesh.subMeshes.emplace_back(materialIndex);
			it = subMeshOf.emplace(materialIndex, outMesh.subMeshes.size() - 1).first;
		}

		CutSubMesh& sub = outMesh.subMeshes[it->second];

		sub.indices.push_back(static_cast<unsigned int>(i0));
		sub.indices.push_back(static_cast<unsigned int>(i1));
		sub.indices.push_back(static_cast<unsigned int>(i2));
	}

	if (outMesh.IsEmpty()) {

		outMesh.Clear();
		return false;
	}

	return true;
}
