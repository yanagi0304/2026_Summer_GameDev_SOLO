#include "EffectTackleMove.h"


EffectTackleMove::EffectTackleMove(const ParameterLoad& parameter, EFFECT_NAME name, const Transform* follow, const Vector3& local):
	EffectBase(parameter)
{
	info.follow = follow;
	info.name = name;
	info.trans.pos = local;

	info.trans.LoadEffect("TackleMove");

	info.trans.angle = GetParameterToVector3("TackleMove", "Angle");
	info.trans.scale = GetParameterToVector3("TackleMove", "Scale");
}