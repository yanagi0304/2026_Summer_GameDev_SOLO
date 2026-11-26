#pragma once

#include <functional>
#include <optional>
#include <string>

#include "../../../Common/Vector3.h"
#include "../../../Manager/Input/InputManager.h"

struct ShooterConfig
{
	// 弾のパラメーターフォルダのパス
	std::string parameterPath;

	// 次の状態へ進めるボタン
	KEY_TYPE advanceKey;

	// ロックオン中の目標の番号を返す関数(発射した瞬間に1回だけ呼ぶ)
	// 空なら追尾しない
	// 目標がいないときは std::nulloptを返す
	std::function<std::optional<int>(void)> getLockOnId;

	// 番号から目標の座標を返す関数(飛んでいる間、毎フレーム呼ぶ)
	// 目標が倒れたときは std::nulloptを返す
	std::function<std::optional<Vector3>(int)> getTargetPos;

	// 既定値
	ShooterConfig(void);

	// アステロイド用の設定
	static ShooterConfig MakeAsteroid(void);

	// ハウンド用の設定
	// getLockOnId: ロックオン中の目標の番号を返す関数
	// getTargetPos: 番号から目標の座標を返す関数
	static ShooterConfig MakeHound(
		const std::function<std::optional<int>(void)>& getLockOnId = nullptr,
		const std::function<std::optional<Vector3>(int)>& getTargetPos = nullptr);
};
