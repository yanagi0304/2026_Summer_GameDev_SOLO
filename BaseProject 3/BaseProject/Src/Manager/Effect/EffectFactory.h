#pragma once

#include "EffectBase.h"
#include "TackleMove/EffectTackleMove.h"

class EffectFactory {

public:

	static EffectBase* CreateEffect(const ParameterLoad& parameter, EFFECT_NAME name, const Transform& trans) {
		EffectBase* res{};

		switch (name){
		default: { break; }
		}
		return res;
	}

	static EffectBase* CreateEffect(const ParameterLoad& parameter, EFFECT_NAME name, const Transform* trans, const Vector3& local) {
		EffectBase* res{};

		switch (name){
		case EFFECT_NAME::TACKLE_MOVE: { res = new EffectTackleMove(parameter, name, trans, local); break; }
		default: { break; }
		}
		return res;
	}
};