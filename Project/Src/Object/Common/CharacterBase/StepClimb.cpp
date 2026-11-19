#include "StepClimb.h"

#include <algorithm>
#include <cmath>

namespace
{
	// 背の高い壁の判定: 登れる高さを少し超えた位置で、障害物と同じ面(ほぼ同じ距離)に当たるか
	constexpr float TALL_WALL_PROBE_MARGIN = 8.0f;
	constexpr float SAME_FACE_TOLERANCE = 8.0f;
}

float ComputeForcedLift(const Vector3& pos, const Vector3& inputDirection, float intendedSpeed, float progress,
	const StepClimbParams& params, const StepRayFunc& ray, StepClimbState& state, StepClimbDebug* debug)
{
	// 結果を記録しながら返す
	float clearance = -1.0f;
	const auto finish = [&](StepClimbResult result, float wallDistance)
	{
		if (debug)
		{
			debug->result = result;
			debug->groundClearance = clearance;
			debug->wallDistance = wallDistance;
			debug->lifted = state.lifted;
			debug->progress = progress;
		}
	};

	// 1) 入力して、目標の速度で進もうとしているか
	Vector3 dir(inputDirection.x, 0.0f, inputDirection.z);
	if (intendedSpeed < 0.5f || dir.LengthSq() < 1.0e-6f)
	{
		state.Reset();
		finish(StepClimbResult::NotPressing, 0.0f);
		return 0.0f;
	}
	dir = dir.Normalized();

	// 2) 前へ進めているか。進めていれば、登る必要はない(押しつけが終わったので、状態を戻す)
	if (progress >= intendedSpeed * params.progressRatio)
	{
		state.Reset();
		finish(StepClimbResult::Progressing, 0.0f);
		return 0.0f;
	}

	// 進めていない状態が、数フレーム続いた時だけ持ち上げる(加速中の一瞬を、押しつけと間違えない)
	if (++state.blockedFrames < params.blockedFramesRequired)
	{
		finish(StepClimbResult::NotBlockedYet, 0.0f);
		return 0.0f;
	}

	// 1回の押しつけで持ち上げられる合計に、上限がある(壁や急な坂を、延々と登り続けないため)
	if (state.lifted >= params.maxHeight - 0.01f)
	{
		finish(StepClimbResult::LimitReached, 0.0f);
		return 0.0f;
	}

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

	// 3) 地面にいるか(直下のレイ)。持ち上げ始めは地面の近くから。持ち上げ中は、持ち上げた分だけ離れていてよい
	//    足元(Transform)は、地面から少し浮いていることがあるので、直下の地面を調べる
	const float allowedClearance = params.maxGroundClearance + state.lifted;
	const StepRayHit self = trace(0, Vector3(pos.x, pos.y + 20.0f, pos.z), Vector3(0.0f, -1.0f, 0.0f), 20.0f + allowedClearance + 30.0f);
	if (self.hit) { clearance = pos.y - self.point.y; }
	if (!self.hit || clearance > allowedClearance)
	{
		finish(StepClimbResult::NotGrounded, 0.0f);
		return 0.0f;
	}
	const float groundNow = self.point.y;

	// 1フレームで進む距離より少し先まで見る(壁に斜めから近づく時は、距離が長くなるので、半径の1.6倍まで見る)
	const float reach = params.radius * 1.6f + (std::max)(intendedSpeed * 2.0f, 15.0f);

	// 4) 前方にステージの障害物があるか。低い高さから順に飛ばして、最初に当たった面を障害物とする
	//    (敵や他のキャラに止められている時に、乗り越えてしまわないため。縁石の下が空いた形状にも対応する)
	StepRayHit low;
	for (int i = 0; i < params.probeCount && i < 5; ++i)
	{
		const float h = params.probeHeights[i];
		if (h >= params.maxHeight) { break; }
		const StepRayHit r = trace(1, Vector3(pos.x, groundNow + h, pos.z), dir, reach);
		if (r.hit) { low = r; break; }
	}
	if (!low.hit)
	{
		finish(StepClimbResult::NoStageObstacle, 0.0f);
		return 0.0f;
	}

	// 5) 背の高い壁なら登らない(登れる高さを少し超えた位置にも、同じ面が続いている)
	//    縁石の後ろに壁があっても、面の距離が違えば、同じ壁とはみなさない
	const StepRayHit high = trace(2, Vector3(pos.x, groundNow + params.maxHeight + TALL_WALL_PROBE_MARGIN, pos.z), dir, reach);
	if (high.hit && std::abs(high.distance - low.distance) <= SAME_FACE_TOLERANCE)
	{
		finish(StepClimbResult::WallTooTall, low.distance);
		return 0.0f;
	}

	// 6) 少しだけ持ち上げる。進めるようになるまで、毎フレーム繰り返される
	const float lift = (std::min)(params.liftPerFrame, params.maxHeight - state.lifted);
	state.lifted += lift;
	finish(StepClimbResult::Lifting, low.distance);
	return lift;
}
