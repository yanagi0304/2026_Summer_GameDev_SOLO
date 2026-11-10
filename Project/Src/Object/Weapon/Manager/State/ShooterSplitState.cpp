#include "ShooterSplitState.h"

#include "../AsteroidSwarm.h"

#include "../../../Common/Transform/Transform.h"

#include "../../../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "../../../../Manager/Input/InputManager.h"

ShooterSplitState::ShooterSplitState(
	const Transform& ownerTrans,
	AsteroidSwarm& swarm,
	const std::function<Vector3(void)>& getCenterPos,
	const std::function<void(bool)>& setAim
) :
	ownerTrans(ownerTrans),
	swarm(swarm),
	getCenterPos(getCenterPos),
	setAim(setAim)
{
}

void ShooterSplitState::Enter(void)
{
	// 27個を出して、立方体の形に並べる
	swarm.ShowFormation(ownerTrans, getCenterPos());
}

void ShooterSplitState::Update(void)
{
	// 手の動きと向きに合わせて並べ直す
	swarm.FollowFormation(ownerTrans, getCenterPos());

	// ズームキーを押している間だけエイム(ズームイン)する
	setAim(Input::GetIns().GetInfo(KEY_TYPE::CameraZoomIn).now);
}

void ShooterSplitState::Exit(void)
{
	// この状態を出るときは、必ずズームを戻す(発射・武器切り替えのどちらでも)
	setAim(false);
}

int ShooterSplitState::GetIdleAnimeID(void) const
{
	return CHARACTER_ANIME::ASTEROID_CREATE_R;
}
