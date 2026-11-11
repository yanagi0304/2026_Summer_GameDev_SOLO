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

	// ロックオン中の目標座標を返す関数
	// 空なら追尾しない
	// 目標がいないときは std::nulloptを返す
	std::function<std::optional<Vector3>(void)> getLockOnPos;

	// 既定値
	ShooterConfig(void);

	// アステロイド用の設定
	static ShooterConfig MakeAsteroid(void);

	// ハウンド用の設定
	// getLockOnPos: 目標の座標を返す関数
	static ShooterConfig MakeHound(const std::function<std::optional<Vector3>(void)>& getLockOnPos = nullptr);
};
