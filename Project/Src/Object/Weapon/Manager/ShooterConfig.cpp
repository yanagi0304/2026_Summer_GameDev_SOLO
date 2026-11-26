#include "ShooterConfig.h"

ShooterConfig::ShooterConfig(void) :
	parameterPath("Data/Parameter/Weapon/Asteroid/"),
	advanceKey(KEY_TYPE::PlayerMain),
	getLockOnId(nullptr),
	getTargetPos(nullptr)
{
}

ShooterConfig ShooterConfig::MakeAsteroid(void)
{
	// 既定値がそのままアステロイド(直進弾)の設定
	return ShooterConfig();
}

ShooterConfig ShooterConfig::MakeHound(
	const std::function<std::optional<int>(void)>& getLockOnId,
	const std::function<std::optional<Vector3>(int)>& getTargetPos)
{
	ShooterConfig config;

	// 専用のパラメーター・モデルができるまでは、アステロイドと同じものを使う
	config.parameterPath = "Data/Parameter/Weapon/Asteroid/";

	// サブ武器用のボタンがまだ無いので、いまはメインと同じボタンにしておく
	config.advanceKey = KEY_TYPE::PlayerSub;

	// ロックオン先を渡すと、追尾弾になる
	config.getLockOnId = getLockOnId;
	config.getTargetPos = getTargetPos;

	return config;
}
