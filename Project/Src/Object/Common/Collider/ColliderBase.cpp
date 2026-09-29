#include "ColliderBase.h"

#include <cassert>

#include "../../../Utility/Utility.h"

CollisionResult::CollisionResult(void) :
	point(),
	normal(),
	penetration(0.0f)
{
}

ColliderBase::ColliderBase(COLLIDER_TAG type, const Vector3& pos, const Vector3& angle) :
	trans(nullptr),
	dynamicFlg(nullptr),
	pushFlg(nullptr),
	pushWeight(nullptr),
	pos(pos),
	angle(angle),
	judgeFlg(true),
	tag(type),
	shape(COLLIDER_SHAPE::None),
	OnCollision(nullptr),
	OnGrounded(nullptr)
{
}

void ColliderBase::SetTransformPtr(Transform* ptr) { trans = ptr; }

void ColliderBase::SetDynamicFlgPtr(const bool* ptr) { dynamicFlg = ptr; }

void ColliderBase::SetPushFlgPtr(const bool* ptr) { pushFlg = ptr; }

void ColliderBase::SetPushWeightPtr(const unsigned char* ptr) { pushWeight = ptr; }

void ColliderBase::SetOnCollisionFunc(std::function<void(COLLIDER_TAG, const ColliderBase&, const CollisionResult&)> OnCollisionFunc) { OnCollision = std::move(OnCollisionFunc); }

void ColliderBase::SetOnGroundedFunc(std::function<void(COLLIDER_TAG, const ColliderBase&)> OnGroundedFunc) { OnGrounded = std::move(OnGroundedFunc); }

Vector3 ColliderBase::GetPos(void)const
{
	assert(trans != nullptr);
	return trans->pos + trans->VTrans(pos);
}

Vector3 ColliderBase::GetPrevPos(void)const
{
	assert(trans != nullptr);
	return trans->prevPos + trans->VTrans(pos);
}

Vector3 ColliderBase::GetAngle(void)const
{
	assert(trans != nullptr);
	return trans->angle + angle;
}

MATRIX ColliderBase::GetAngleMat(void)const
{
	assert(trans != nullptr);
	return MatrixAllMultXZY({ trans->angle,angle });
}

Vector3 ColliderBase::VTrans(const Vector3& v)const
{
	return (v != 0.0f) ? Vector3(VTransform(v.ToVECTOR(), GetAngleMat())) : Vector3();
}

const Transform& ColliderBase::GetTransform(void)const
{
	assert(trans != nullptr);
	return *trans;
}

bool ColliderBase::GetDynamicFlg(void)const { return (dynamicFlg != nullptr) ? *dynamicFlg : true; }

bool ColliderBase::GetJudgeFlg(void)const { return judgeFlg; }

bool ColliderBase::GetPushFlg(void)const { return (pushFlg != nullptr) ? *pushFlg : true; }

unsigned char ColliderBase::GetPushWeight(void)const { return (pushWeight != nullptr) ? *pushWeight : 0; }

COLLIDER_TAG ColliderBase::GetTag(void)const { return tag; }

COLLIDER_SHAPE ColliderBase::GetShape(void)const { return shape; }

void ColliderBase::CallOnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	if (!OnCollision) { return; }
	OnCollision(ownTag, other, result);
}

void ColliderBase::CallOnGrounded(COLLIDER_TAG ownTag, const ColliderBase& other)
{
	if (!OnGrounded) { return; }
	OnGrounded(ownTag, other);
}

void ColliderBase::SetTransformPos(const Vector3& pos)
{
	assert(trans != nullptr);
	trans->pos = pos;
}

void ColliderBase::SetTransformPosAdd(const Vector3& vec)
{
	assert(trans != nullptr);
	trans->pos += vec;
}

void ColliderBase::SetJudgeFlg(bool flg) { judgeFlg = flg; }

void ColliderBase::SetShape(COLLIDER_SHAPE s) { shape = s; }
