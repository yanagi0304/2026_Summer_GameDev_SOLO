#include "SlashTracker.h"

SlashTracker::SlashTracker(void) :
	updateCount(0),
	base(),
	tip(),
	prevTip()
{
}

void SlashTracker::Reset(void)
{
	updateCount = 0;
}

void SlashTracker::Update(const Vector3& base, const Vector3& tip)
{
	// 2回目以降は、直前の先端を「前フレームの先端」として残す
	if (updateCount > 0) { prevTip = this->tip; }

	this->base = base;
	this->tip = tip;

	// 数え過ぎてオーバーフローしないよう2で止める
	if (updateCount < 2) { updateCount++; }
}

bool SlashTracker::GetPlane(CutPlane& outPlane) const
{
	// 前フレームの先端が無い
	if (updateCount < 2) { return false; }

	// 刃の向きと、先端の移動方向
	const Vector3 bladeDir = tip - base;
	const Vector3 move = tip - prevTip;

	const float bladeLen = bladeDir.Length();
	const float moveLen = move.Length();

	// 刃が無い、または先端が止まっている
	if (bladeLen <= 0.0001f || moveLen < MIN_MOVE) { return false; }

	// 面の法線（刃の向き × 移動方向）
	const Vector3 normal = bladeDir.Cross(move);

	// 突き（刃と移動方向がほぼ平行）だと面が定まらない
	if (normal.Length() < MIN_SIN * bladeLen * moveLen) { return false; }

	// 刃の根本を通る面（刃の線全体が面に乗る）
	outPlane = CutPlane(base, normal);

	return true;
}
