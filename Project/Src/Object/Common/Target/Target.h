#pragma once

#include <vector>
#include "../../../Common/Vector3.h"
#include <optional>

class ActorBase;

class Target
{
public:

	Target();

	~Target() = default;

	void Update(void);

	void Draw(void);

	// 現在のターゲットを取得する
	ActorBase* GetTarget(void) const { return target; }

	//ターゲットがいるかどうか
	bool HasTarget(void) const { return target != nullptr; }

	//ターゲットの座標を取得する
	const Vector3& GetTargetPos(void) const { return targetPos; }

	//ターゲットを解除する
	void Release(void) { target = nullptr; }

	// ターゲット候補を増やす
	void IncludeSelfInTargets(ActorBase* actor) 
	{ if (actor != nullptr) { candidates.push_back(actor); } }

	// ターゲットを使う側
	void SetTargetActor(const Vector3& pos) { ownerPos = std::cref(pos); }

private:

	// ターゲット候補リスト
	std::vector<ActorBase*> candidates;

	// 撃つ側の座標
	std::optional<std::reference_wrapper<const Vector3>> ownerPos;

	// 現在のターゲット
	ActorBase* target;

	// ターゲットの座標
	Vector3 targetPos;

	// ターゲット候補とする最大距離
	float maxDistance;

	// 画面中心から許容する範囲
	float minDot;

	// 高さの補正
	float aimHeightOffset;

	// カメラの角度から正面方向のベクトルを求める
	Vector3 CalcCameraForward(void) const;

};

