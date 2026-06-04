#pragma once
#include "../ActorBase.h"
class Stage :
    public ActorBase
{
public:

    Stage(const std::string& parameterPath);
    ~Stage() = default;
	void Load(void) override;

};

