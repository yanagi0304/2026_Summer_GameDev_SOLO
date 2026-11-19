#pragma once

#include <vector>

#include "../../Common/CharacterBase/CharacterStateBase.h"

#include "../../../Common/Vector3.h"

#include "../Navigation/AStar.h"

class NavManager;
class GroundMotionController;

// 敵の追跡状態
// 一定間隔で目標までの経路を(NavManager経由で)探索し直し、経路上の次の点へ向かって歩く
// 周りの敵とは、重ならないように少し離れる
class EnemyChaseState : public CharacterStateBase
{
public:

	// 追跡の設定
	struct Params
	{
		// 移動速度
		float moveSpeed = 7.0f;

		// 経路を探索し直す間隔(フレーム)。実際は少しばらつかせる
		float repathFrames = 24.0f;

		// 経路上の点に着いたとみなす距離
		float arriveDistance = 60.0f;

		// 目標にこれ以上近づかない距離(攻撃実装までの仮の値)
		float stopDistance = 150.0f;

		// 周りの敵から離れる力の強さ(進行方向を1とした時の比率)
		float separationWeight = 1.0f;
	};

	EnemyChaseState(
		const Params& params,
		GroundMotionController& motion,
		NavManager& nav,
		std::function<Vector3(void)> getPos,
		std::function<Vector3(void)> getGoal,
		std::function<bool(void)> shouldChase,
		std::function<Vector3(void)> getSeparation,
		std::function<void(void)> playAnimeWalk
	);

	// 未処理の探索依頼を取り消す(消えた後に結果が届かないようにする)
	~EnemyChaseState()override;

	// 追跡する理由があれば、自分の状態に遷移
	void OwnStateConditionUpdate(void)override;

	void Enter(void)override;
	void Update(void)override;
	void Exit(void)override;

	// 現在の経路(デバッグ表示用)
	const std::vector<PathPoint>& GetPath(void)const { return path; }

	// 次に向かう経路上の点の番号(デバッグ表示用)
	size_t GetPathIndex(void)const { return pathIndex; }

private:

	// 水平距離
	static float DistanceXZ(const Vector3& a, const Vector3& b);

	// 経路の探索を依頼する(結果は後のフレームに届く)
	void RequestRepath(const Vector3& pos, const Vector3& goal);

	const Params params;

	GroundMotionController& motion;
	NavManager& nav;

	const std::function<Vector3(void)> getPos;
	const std::function<Vector3(void)> getGoal;
	const std::function<bool(void)> shouldChase;
	const std::function<Vector3(void)> getSeparation;
	const std::function<void(void)> playAnimeWalk;

	// 現在の経路と、次に向かう点の番号
	std::vector<PathPoint> path;
	size_t pathIndex;

	// 次の再探索までの残りフレーム
	float repathTimer;
};
