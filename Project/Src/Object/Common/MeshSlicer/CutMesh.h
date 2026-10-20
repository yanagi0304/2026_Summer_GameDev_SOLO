#pragma once

#include <vector>

// Vector3.h の場所はプロジェクトに合わせて調整する
#include "../../../Common/Vector3.h"

// 切断用の頂点
struct CutVertex
{
	// 座標
	Vector3 pos;

	// 法線
	Vector3 normal;

	// UV
	float u, v;

	// 頂点カラー
	COLOR_F color;

	CutVertex(void) :
		pos(),
		normal(),
		u(0.0f), v(0.0f),
		color{ 1.0f, 1.0f, 1.0f, 1.0f }
	{
	}
};

// マテリアル1つ分の三角形リスト
struct CutSubMesh
{
	// 元モデルのマテリアル番号
	int materialIndex;

	// 3つ1組で1枚の三角形
	std::vector<unsigned int> indices;

	CutSubMesh(void) : materialIndex(0), indices() {}
	explicit CutSubMesh(int materialIndex) : materialIndex(materialIndex), indices() {}
};

// 切断対象になるメッシュ
struct CutMesh
{
	// 頂点配列（全サブメッシュで共有）
	std::vector<CutVertex> vertices;

	// マテリアルごとの三角形リスト
	std::vector<CutSubMesh> subMeshes;

	// 三角形の総数
	size_t TriangleNum(void) const {
		size_t num = 0;
		for (const CutSubMesh& sub : subMeshes) { num += sub.indices.size() / 3; }
		return num;
	}

	// 三角形が1枚も無いか
	bool IsEmpty(void) const { return TriangleNum() == 0; }

	// 中身を空にする
	void Clear(void) {
		vertices.clear();
		subMeshes.clear();
	}

	// MeshCollider::Setup(vertices, indices) に渡せる形へ変換する
	void ToPositionsAndIndices(std::vector<Vector3>& outPositions, std::vector<unsigned int>& outIndices) const;

	// 動作確認用の箱メッシュ（原点中心）を作る
	static CutMesh MakeBox(const Vector3& size);

	// 動作確認用のワイヤー描画（offsetで全体をずらして描画できる）
	void DrawWire(unsigned int color, const Vector3& offset = Vector3()) const;
};
