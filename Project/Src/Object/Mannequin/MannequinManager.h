#pragma once
#include "../Common/ActorBase/ActorBase.h"

class MannequinManager :
    public ActorBase
{
public:
	MannequinManager();

	~MannequinManager()override = default;

	void Load(void)override;
};

