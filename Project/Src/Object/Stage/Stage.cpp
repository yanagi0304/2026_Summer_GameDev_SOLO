#include "Stage.h"

Stage::Stage(const std::string& parameterPath)
{
}

void Stage::Load(void)
{
	trans.Load("Stage/stage");
}
