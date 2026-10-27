#pragma once
#include "../Common/ActorBase/ActorBase.h"

class MannequinManager :
    public ActorBase
{
public:
	MannequinManager();

	~MannequinManager()override = default;

	void Load(void)override;

	// ƒ^[ƒQƒbƒg‚Æ‚µ‚Ä‚Ìw‰c
	TARGET_TEAM GetTargetTeam(void) const override { return TARGET_TEAM::Enemy; }
};

