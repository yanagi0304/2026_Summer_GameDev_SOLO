#pragma once
#include "../../ActorBase.h"
class StageTutrial :
    public ActorBase
{
public:
    StageTutrial();
    StageTutrial(const std::string& parameterPath);
    ~StageTutrial() = default;
	void Load(void) override;
    void SubInit(void) override;
};

