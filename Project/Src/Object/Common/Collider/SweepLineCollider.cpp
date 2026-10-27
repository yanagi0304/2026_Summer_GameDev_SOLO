#include "SweepLineCollider.h"

#include "../Collider/LineCollider.h"

SweepLineCollider::SweepLineCollider(const Vector3& pos)
	:currPos(pos),
	prevPos(currPos)
{
}

void SweepLineCollider::Load(void)
{
	AddCollider(new LineCollider(COLLIDER_TAG::DebugObject, currPos, prevPos));
}

void SweepLineCollider::SubUpdate(void)
{
	//前回の座標を前フレーム座標に送る
	prevPos = currPos;
}

void SweepLineCollider::SubDraw(void)
{
}
