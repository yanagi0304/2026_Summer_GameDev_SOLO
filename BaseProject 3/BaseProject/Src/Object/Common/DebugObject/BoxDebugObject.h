#pragma once

#include "../../ActorBase.h"

#include "../../../Manager/Input/KeyManager.h"
#include "../Collider/BoxCollider.h"

class BoxDebugObject : public ActorBase
{
public:
	BoxDebugObject(const Vector3& size) : ActorBase(), size(size) {}
	~BoxDebugObject()override = default;

	void Load(void)override {
		ColliderCreate(new BoxCollider(COLLIDER_TAG::SPHERE_DEBUG_OBJECT, size, size.Length() * 0.5f));
	}

private:
	void SubInit(void)override {
		SetDynamicFlg(false);
		trans.pos = Vector3();
	}

	Vector3 size;
};