#include "ShooterCubeState.h"

#include "../AsteroidSwarm.h"

#include "../../../Common/AnimationController/CharacterAnimeTypeDefine.h"

ShooterCubeState::ShooterCubeState(AsteroidSwarm& swarm, const std::function<Vector3(void)>& getCenterPos) :
	swarm(swarm),
	getCenterPos(getCenterPos)
{
}

void ShooterCubeState::Enter(void)
{
	// 1個だけ手元に出す
	swarm.ShowOne(getCenterPos());
}

void ShooterCubeState::Update(void)
{
	// 手の動きに合わせて追従させる
	swarm.FollowOne(getCenterPos());
}

int ShooterCubeState::GetIdleAnimeID(void) const
{
	return CHARACTER_ANIME::ASTEROID_CREATE_R;
}
