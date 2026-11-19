#include "EnemyChaseState.h"

#include <cmath>
#include <utility>

#include "../../../Manager/TimeScale/TimeScale.h"

#include "../../Common/MotionController/GroundMotionController.h"

#include "../Navigation/NavGrid.h"

EnemyChaseState::EnemyChaseState(
	const Params& params,
	GroundMotionController& motion,
	const NavGrid& grid,
	AStar& astar,
	std::function<Vector3(void)> getPos,
	std::function<Vector3(void)> getGoal,
	std::function<bool(void)> shouldChase,
	std::function<void(void)> playAnimeWalk
) :
	params(params),
	motion(motion),
	grid(grid),
	astar(astar),
	getPos(getPos),
	getGoal(getGoal),
	shouldChase(shouldChase),
	playAnimeWalk(playAnimeWalk),
	path(),
	pathIndex(0),
	repathTimer(0.0f)
{
}

void EnemyChaseState::OwnStateConditionUpdate(void)
{
	// 追跡する理由があれば追跡へ
	if (shouldChase()) { OwnChangeState(); }
}

void EnemyChaseState::Enter(void)
{
	motion.SetMoveSpeed(params.moveSpeed);
	playAnimeWalk();

	// 入ったらすぐ探索する
	path.clear();
	pathIndex = 0;
	repathTimer = 0.0f;
}

void EnemyChaseState::Update(void)
{
	const Vector3 pos = getPos();
	const Vector3 goal = getGoal();

	// 一定間隔で経路を探索し直す
	repathTimer -= TimeScale::Get();
	if (repathTimer <= 0.0f)
	{
		Repath(pos, goal);
		repathTimer = params.repathFrames;
	}

	// 目標に十分近ければそれ以上近づかない
	if (DistanceXZ(pos, goal) < params.stopDistance) { return; }

	// 着いた点は飛ばして、次に向かう点を決める
	while (pathIndex < path.size() && DistanceXZ(pos, path[pathIndex].pos) < params.arriveDistance)
	{
		++pathIndex;
	}
	if (pathIndex >= path.size()) { return; }

	// 次の点へ向かう(経路上の動作は今は歩くだけ)
	Vector3 dir = path[pathIndex].pos - pos;
	dir.y = 0.0f;
	if (dir.LengthSq() < 1.0e-6f) { return; }

	motion.ApplyInput(dir.Normalized());
}

void EnemyChaseState::Exit(void)
{
	path.clear();
	pathIndex = 0;
}

float EnemyChaseState::DistanceXZ(const Vector3& a, const Vector3& b)
{
	const float dx = a.x - b.x;
	const float dz = a.z - b.z;
	return std::sqrt(dx * dx + dz * dz);
}

void EnemyChaseState::Repath(const Vector3& pos, const Vector3& goal)
{
	std::vector<PathPoint> newPath;
	if (astar.FindPath(grid, pos, goal, newPath))
	{
		path = std::move(newPath);
	}
	else
	{
		// 探索に失敗したら立ち止まる(屋上など、地上から届かない場所にいる場合など)
		path.clear();
	}
	pathIndex = 0;
}
