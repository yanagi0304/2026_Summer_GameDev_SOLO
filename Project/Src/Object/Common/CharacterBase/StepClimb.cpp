#include "StepClimb.h"

#include <algorithm>
#include <cmath>

namespace
{
	// 法線のYがこれ以上なら、坂・床(歩いて上れる面)とみなす
	constexpr float FLOOR_NORMAL_Y = 0.7f;

	// 壁の面から、段差の上面を調べる位置までの距離(面のすぐ奥)
	constexpr float PAST_FACE = 8.0f;

	// 同じ面とみなす距離の差(面取りされた縁石のように、少し傾いた面も同じ面として追えるようにする)
	constexpr float FACE_TOLERANCE = 8.0f;

	// 足元の面が、これより水平に近ければ、緩い坂とみなして物理に任せる(法線のY)
	constexpr float SLOPE_FACE_NORMAL_Y = 0.85f;

	// 面の高さを二分探索する回数
	constexpr int FACE_SEARCH_ITERATIONS = 6;
}

float ComputeStepRise(const Vector3& pos, const Vector3& moveDirection, float speed, const StepClimbParams& params, const StepRayFunc& ray, StepClimbDebug* debug)
{
	// 結果を記録しながら返す
	float clearance = -1.0f;
	const auto finish = [debug, &clearance](StepClimbResult result, float wallDistance, float rise)
	{
		if (debug) { debug->result = result; debug->wallDistance = wallDistance; debug->rise = rise; debug->groundClearance = clearance; }
	};

	// ほとんど動いていなければ、登る必要はない
	if (speed < 0.5f) { finish(StepClimbResult::NotMoving, 0.0f, 0.0f); return 0.0f; }

	Vector3 dir(moveDirection.x, 0.0f, moveDirection.z);
	if (dir.LengthSq() < 1.0e-6f) { finish(StepClimbResult::NotMoving, 0.0f, 0.0f); return 0.0f; }
	dir = dir.Normalized();

	// 判定を行う時だけ、記録したレイを入れ替える(止まっている間は、直前の分が残る)
	if (debug) { debug->probes.clear(); }

	// レイを飛ばし、デバッグ用に、飛ばしたレイと結果を記録する(kindは描画の色分け用)
	const auto trace = [&](int kind, const Vector3& origin, const Vector3& direction, float maxDistance)
	{
		const StepRayHit result = ray(origin, direction, maxDistance);
		if (debug)
		{
			StepClimbDebug::Probe probe;
			probe.from = origin;
			probe.to = result.hit ? result.point : origin + direction.Normalized() * maxDistance;
			probe.hit = result.hit;
			probe.kind = kind;
			debug->probes.push_back(probe);
		}
		return result;
	};

	// 1フレームで進む距離より少し先まで見る
	// (壁に斜めから近づく時は、壁までの距離が、真正面より長くなるので、半径の1.6倍まで見る)
	const float reach = params.radius * 1.6f + (std::max)(speed * 2.0f, 15.0f);

	// 1) 地面にいるか。足元(Transform)は、地面から少し浮いていることがあるので、直下の地面を調べる
	//    接地フラグは使わない(段差に押しつけて歩くと、角に押し上げられて、フラグが外れる)
	const StepRayHit self = trace(0, Vector3(pos.x, pos.y + 20.0f, pos.z), Vector3(0.0f, -1.0f, 0.0f), 20.0f + params.maxGroundClearance + 30.0f);
	if (self.hit) { clearance = pos.y - self.point.y; }
	if (!self.hit || clearance > params.maxGroundClearance)
	{
		finish(StepClimbResult::NotGrounded, 0.0f, 0.0f);
		return 0.0f;
	}
	const float groundNow = self.point.y;

	// 2) 進行方向に壁(縁石)があるか。低い高さから順に飛ばして、最初に当たった高さを面とする
	//    縁石の下が空いている形状では、低い高さのレイは、面の下をくぐり抜けてしまうため
	StepRayHit low;
	float lowHeight = 0.0f;
	for (int i = 0; i < params.probeCount && i < 5; ++i)
	{
		const float h = params.probeHeights[i];
		if (h >= params.maxHeight) { break; }
		const StepRayHit r = trace(1, Vector3(pos.x, groundNow + h, pos.z), dir, reach);
		if (r.hit) { low = r; lowHeight = h; break; }
	}
	if (!low.hit) { finish(StepClimbResult::NoWallAhead, 0.0f, 0.0f); return 0.0f; }

	// 当たった面が緩い坂なら、物理に任せる(急な面取りの縁石は、段差として扱う)
	if (std::abs(low.normal.y) >= SLOPE_FACE_NORMAL_Y) { finish(StepClimbResult::SlopeAhead, low.distance, 0.0f); return 0.0f; }

	// 3) 壁の面が、どの高さまで続くかを調べる
	//    指定の高さで、低いレイと同じ面(ほぼ同じ距離)に当たるか
	const auto onSameFace = [&](float height)
	{
		const StepRayHit r = trace(2, Vector3(pos.x, groundNow + height, pos.z), dir, reach);
		return r.hit && std::abs(r.distance - low.distance) <= FACE_TOLERANCE;
	};

	// 登れる高さを少し超えた位置にも同じ面が続いていれば、背の高い壁なので登れない
	const float limit = params.maxHeight + PAST_FACE;
	if (onSameFace(limit)) { finish(StepClimbResult::WallTooTall, low.distance, 0.0f); return 0.0f; }

	// 面が続く一番高い位置を、二分探索で求める(上面の探索を、面のすぐ上から始めるため)
	// 高い位置から降ろすと、縁石の上の庇や張り出しに先に当たってしまう
	float lo = lowHeight;
	float hi = limit;
	for (int i = 0; i < FACE_SEARCH_ITERATIONS; ++i)
	{
		const float mid = (lo + hi) * 0.5f;
		if (onSameFace(mid)) { lo = mid; } else { hi = mid; }
	}
	const float faceTop = hi;   // 面が終わっている高さ(の上限)

	// 4) 壁のすぐ奥で、面のすぐ上から下へレイを飛ばして、段差の上面の高さを調べる
	const Vector3 ahead = pos + dir * (low.distance + PAST_FACE);
	const float startHeight = faceTop + 2.0f;
	const StepRayHit top = trace(3, Vector3(ahead.x, groundNow + startHeight, ahead.z), Vector3(0.0f, -1.0f, 0.0f), startHeight + 15.0f);
	if (!top.hit || std::abs(top.normal.y) < FLOOR_NORMAL_Y) { finish(StepClimbResult::NoTopSurface, low.distance, 0.0f); return 0.0f; }

	const float rise = top.point.y - groundNow;
	if (rise < params.minHeight || rise > params.maxHeight) { finish(StepClimbResult::RiseOutOfRange, low.distance, rise); return 0.0f; }

	// 5) 段差の上に立った時、頭上に天井や張り出しがないか
	const StepRayHit ceiling = trace(4, Vector3(ahead.x, top.point.y + 2.0f, ahead.z), Vector3(0.0f, 1.0f, 0.0f), params.bodyHeight);
	if (ceiling.hit) { finish(StepClimbResult::Ceiling, low.distance, rise); return 0.0f; }

	finish(StepClimbResult::Climbed, low.distance, rise);
	return rise;
}
