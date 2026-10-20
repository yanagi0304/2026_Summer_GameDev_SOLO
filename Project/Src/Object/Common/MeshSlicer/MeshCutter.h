#pragma once

#include "CutMesh.h"

// 切断面（平面）
struct CutPlane
{
	// 平面上の1点
	Vector3 point;

	// 法線（表側の向き。コンストラクタで正規化される）
	Vector3 normal;

	CutPlane(void) : point(), normal(Vector3::Yonly(1.0f)) {}

	CutPlane(const Vector3& point, const Vector3& normal) :
		point(point),
		normal(normal.Normalized())
	{
	}

	// 3点を通る平面を作る（法線は (b-a) x (c-a) の向き）
	static CutPlane FromPoints(const Vector3& a, const Vector3& b, const Vector3& c) {
		return CutPlane(a, (b - a).Cross(c - a));
	}

	// 点から平面までの符号付き距離（正 = 表側、負 = 裏側）
	float Distance(const Vector3& p) const { return (p - point).Dot(normal); }
};

// メッシュを平面で2つに分ける（DxLibやActorには依存しない）
class MeshCutter
{
public:

	/// <summary>
	/// メッシュを平面で切断し、切り口にふた（マテリアル番号 -1・テクスチャなし）を付ける
	/// ふたが付くのは、切り口が閉じた輪になる部分だけ（穴あきの輪や、途切れた輪は埋めない）
	/// </summary>
	/// <param name="src">切断元（front/backと同じオブジェクトは指定不可）</param>
	/// <param name="plane">切断面</param>
	/// <param name="front">（out）法線側のメッシュ</param>
	/// <param name="back">（out）法線の反対側のメッシュ</param>
	/// <returns>true = 両側に三角形ができた（実際に切れた）、false = 切れなかった</returns>
	static bool Cut(const CutMesh& src, const CutPlane& plane, CutMesh& front, CutMesh& back);

private:

	MeshCutter(void) = delete;
};
