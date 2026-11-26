#pragma once
#include "../../Common/ActorBase/ActorBase.h"
class StageTutrial :
    public ActorBase
{
public:
    StageTutrial();
    ~StageTutrial() = default;
	void Load(void) override;
    void SubInit(void) override;

    ACTOR_DRAW_TYPE DefaultDrawType(void)const override { return ACTOR_DRAW_TYPE::Default; }

};