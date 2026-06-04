#pragma once

#include "../EffectBase.h"

class EffectTackleMove : public EffectBase {
public:

	EffectTackleMove(const ParameterLoad& parameter, EFFECT_NAME name, const Transform* follow, const Vector3& local);

	~EffectTackleMove() = default;
};