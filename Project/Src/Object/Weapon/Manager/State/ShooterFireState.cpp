#include "ShooterFireState.h"

#include "../AsteroidSwarm.h"

#include "../../../Common/Transform/Transform.h"

#include "../../../Common/AnimationController/CharacterAnimeTypeDefine.h"

ShooterFireState::ShooterFireState(const Transform& ownerTrans, AsteroidSwarm& swarm,
	const std::function<std::optional<Vector3>(void)>& getAimDir) :
	ownerTrans(ownerTrans),
	swarm(swarm),
	fireDir(Vector3::Zonly(1.0f)),
	getAimDir(getAimDir)
{
}

void ShooterFireState::Enter(void)
{
	// 発射方向を決めてから、全部撃つ
	UpdateFireDir();

	swarm.FireAll(fireDir);
}

void ShooterFireState::Exit(void)
{
	// 飛んでいる弾を止めて、速度と軌跡を消す
	swarm.RecallAll();
}

int ShooterFireState::GetIdleAnimeID(void) const
{
	return CHARACTER_ANIME::ASTEROID_IDLE_R;
}

void ShooterFireState::UpdateFireDir(void)
{
	//エイム中はカメラの向きへ撃つ
	const std::optional<Vector3> aimDir = getAimDir();
	if (aimDir.has_value()) {
		fireDir = aimDir.value();
		return;
	}

	// 持ち主の正面方向
	Vector3 forward = ownerTrans.VTrans(Vector3(0.0f, 0.0f, 1.0f));

	// 上下は使わず水平方向だけにする
	forward.y = 0.0f;

	// 長さが0に近いときは、前回の方向の向きのままにする
	if (forward.LengthSq() > 0.000001f) { fireDir = forward.Normalized(); }

}
