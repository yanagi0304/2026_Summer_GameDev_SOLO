#include "EnemyChaseState.h"

#include <cmath>
#include <random>
#include <utility>

#include "../../../Manager/TimeScale/TimeScale.h"

#include "../../Common/MotionController/GroundMotionController.h"

#include "../Navigation/NavManager.h"

namespace
{
	// 再探索のタイミングをずらすための乱数(全員が同じフレームに探索を依頼しないように)
	float RandomFrames(float maxFrames)
	{
		static std::mt19937 rng{ std::random_device{}() };
		return std::uniform_real_distribution<float>(0.0f, maxFrames)(rng);
	}
}

EnemyChaseState::EnemyChaseState(
	const Params& params,
	GroundMotionController& motion,
	NavManager& nav,
	std::function<Vector3(void)> getPos,
	std::function<Vector3(void)> getGoal,
	std::function<bool(void)> shouldChase,
	std::function<Vector3(void)> getSeparation,
	std::function<void(void)> playAnimeWalk
) :
	params(params),
	motion(motion),
	nav(nav),
	getPos(getPos),
	getGoal(getGoal),
	shouldChase(shouldChase),
	getSeparation(getSeparation),
	playAnimeWalk(playAnimeWalk),
	path(),
	pathIndex(0),
	repathTimer(0.0f)
{
}

EnemyChaseState::~EnemyChaseState()
{
	nav.Cancel(this);
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

	// 入ったらすぐ探索を依頼する
	path.clear();
	pathIndex = 0;
	repathTimer = 0.0f;
}

void EnemyChaseState::Update(void)
{
	const Vector3 pos = getPos();
	const Vector3 goal = getGoal();

	// 一定間隔で経路の探索を依頼する(結果が届くまでは、今の経路で歩き続ける)
	repathTimer -= TimeScale::Get();
	if (repathTimer <= 0.0f)
	{
		RequestRepath(pos, goal);
		repathTimer = params.repathFrames + RandomFrames(params.repathFrames * 0.5f);
	}

	// 周りの敵から離れる向き
	const Vector3 separation = getSeparation() * params.separationWeight;

	// 目標に十分近ければそれ以上近づかず、周りの敵と重ならないように広がるだけにする
	if (DistanceXZ(pos, goal) < params.stopDistance)
	{
		if (separation.LengthSq() > 1.0e-4f) { motion.ApplyInput(separation * 0.5f); }
		return;
	}

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

	// 進行方向に、周りの敵から離れる向きを足す
	dir = dir.Normalized() + separation;
	if (dir.LengthSq() < 1.0e-6f) { return; }

	motion.ApplyInput(dir.Normalized());
}

void EnemyChaseState::Exit(void)
{
	nav.Cancel(this);
	path.clear();
	pathIndex = 0;
}

float EnemyChaseState::DistanceXZ(const Vector3& a, const Vector3& b)
{
	const float dx = a.x - b.x;
	const float dz = a.z - b.z;
	return std::sqrt(dx * dx + dz * dz);
}

void EnemyChaseState::RequestRepath(const Vector3& pos, const Vector3& goal)
{
	nav.RequestPath(this, pos, goal,
		[this](bool found, std::vector<PathPoint>& newPath)
		{
			if (found)
			{
				path = std::move(newPath);
			}
			else
			{
				// 探索に失敗したら立ち止まる(屋上など、地上から届かない場所にいる場合など)
				path.clear();
			}
			pathIndex = 0;
		});
}
