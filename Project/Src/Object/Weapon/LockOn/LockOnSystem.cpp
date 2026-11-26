#include "LockOnSystem.h"

#include <cmath>

#include "../../../pch.h"

#include "../../../Manager/Camera/CurrentCamera.h"
#include "../../../Manager/Camera/CameraBase.h"
#include "../../../Manager/Collision/CollisionManager.h"
#include "../../../Manager/Collision/CurrentCollisionManager.h"

void LockOnSystem::Update(bool selecting)
{
	// 対象の座標は毎フレーム作り直す(倒れた対象は一覧から消える)
	candidates_.clear();
	if (collect_) { collect_(candidates_); }

	targetId_ = selecting ? SelectTarget() : std::nullopt;
}

void LockOnSystem::DrawMarker(void) const
{
	if (!targetId_.has_value()) { return; }

	const std::optional<Vector3> pos = GetPos(targetId_.value());
	if (!pos.has_value()) { return; }

	const VECTOR screen = ConvWorldPosToScreenPos(pos.value().ToVECTOR());

	// カメラの後ろにある対象は描かない
	if (screen.z <= 0.0f || screen.z >= 1.0f) { return; }

	const int x = static_cast<int>(screen.x);
	const int y = static_cast<int>(screen.y);

	// 呼び出し元の描画設定を覚えておく
	int blendMode = DX_BLENDMODE_NOBLEND;
	int blendParam = 0;
	GetDrawBlendMode(&blendMode, &blendParam);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawCircle(x, y, MARKER_RADIUS, 0xff4040, FALSE, MARKER_THICKNESS);
	DrawCircle(x, y, MARKER_THICKNESS, 0xff4040, TRUE);

	SetDrawBlendMode(blendMode, blendParam);
}

std::optional<Vector3> LockOnSystem::GetPos(int id) const
{
	for (const LockCandidate& candidate : candidates_) {
		if (candidate.id == id) { return candidate.pos; }
	}

	return std::nullopt;
}

std::optional<int> LockOnSystem::SelectTarget(void) const
{
	const CameraBase& camera = CurrentCamera::Get();
	const Vector3 camPos = camera.GetPos();
	const Vector3 camForward = camera.GetRotation().Rotate(Vector3(0.0f, 0.0f, 1.0f));

	// 正面との内積が1に近いほど、画面の中央にいる
	float bestDot = std::cos(LOCK_ANGLE_DEG * DX_PI_F / 180.0f);
	std::optional<int> result;

	for (const LockCandidate& candidate : candidates_) {

		const Vector3 toTarget = candidate.pos - camPos;
		const float distance = toTarget.Length();
		if (distance > LOCK_RANGE || distance < 1.0e-3f) { continue; }

		const float dot = camForward.Dot(toTarget / distance);
		if (dot <= bestDot) { continue; }

		// レイキャストは重いので、候補になった対象だけ調べる
		if (IsBlocked(camPos, candidate.pos)) { continue; }

		bestDot = dot;
		result = candidate.id;
	}

	return result;
}

bool LockOnSystem::IsBlocked(const Vector3& from, const Vector3& to) const
{
	if (!CurrentCollisionManager::Exists()) { return false; }

	const Vector3 toTarget = to - from;
	const float distance = toTarget.Length();

	RaycastHit hit;
	return CurrentCollisionManager::Get().RaycastStage(from, toTarget / distance, distance, hit);
}
