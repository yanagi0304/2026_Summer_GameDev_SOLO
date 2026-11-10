#include "ShooterIdleState.h"

#include "../AsteroidSwarm.h"

#include "../../../Common/AnimationController/CharacterAnimeTypeDefine.h"

ShooterIdleState::ShooterIdleState(AsteroidSwarm& swarm) :
	swarm(swarm)
{
}

void ShooterIdleState::Enter(void)
{
	// 弾をすべて消す
	swarm.HideAll();
}

int ShooterIdleState::GetIdleAnimeID(void) const
{
	return CHARACTER_ANIME::ASTEROID_IDLE_R;
}
