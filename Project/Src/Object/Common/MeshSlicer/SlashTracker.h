#pragma once

#include "CutMesh.h"
#include "MeshCutter.h"

// 刃の根本と先端の動きから、振った軌跡の面（CutPlane）を作る
// DxLibやActorには依存しない
class SlashTracker
{
public:

	SlashTracker(void);

	// 履歴を消す（攻撃判定の開始時に呼ぶ）
	void Reset(void);

	// 毎フレーム、今の刃の根本と先端（ワールド座標）を渡す
	void Update(const Vector3& base, const Vector3& tip);

	/// <summary>
	/// 軌跡の面を作る（刃の線と、先端の移動方向を含む面）
	/// </summary>
	/// <param name="outPlane">（out）ワールド座標の切断面</param>
	/// <returns>
	/// 作れたらtrue
	/// 履歴が足りない・先端が止まっている・突き（移動方向が刃と同じ向き）のときはfalse
	/// </returns>
	bool GetPlane(CutPlane& outPlane) const;

private:

	// 先端の移動量がこれ未満なら、止まっているとみなす
	static constexpr float MIN_MOVE = 0.5f;

	// 刃と移動方向のなす角のsinがこれ未満（約11度未満）なら、突きとみなす
	// 振り下ろしや横薙ぎは、ほぼ垂直（sin≒1）に動くので影響しない
	static constexpr float MIN_SIN = 0.2f;

	// これまでにUpdateされた回数（2以上で前フレームの先端が使える）
	int updateCount;

	// 今の刃の根本・先端
	Vector3 base;
	Vector3 tip;

	// 前フレームの先端
	Vector3 prevTip;
};
