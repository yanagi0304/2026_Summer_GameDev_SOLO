#pragma once

#include <functional>
#include <optional>
#include <string>

#include "../../../Common/Vector3.h"
#include "../../../Manager/Input/InputManager.h"

// 撃つ武器の設定(Config:コンフィグ=設定)
// アステロイドとハウンドは仕組みが同じなので、違いをここだけで表す
struct ShooterConfig
{
	// 弾のパラメーターフォルダのパス
	std::string parameterPath;

	// 次の状態へ進めるボタン
	KEY_TYPE advanceKey;

	// ロックオン中の目標座標を返す関数(LockOn:ロックオン=狙いを定める)
	// 空なら追尾しない(まっすぐ飛ぶ)
	// 目標がいないときは std::nullopt(ヌルオプト=値なし)を返す
	std::function<std::optional<Vector3>(void)> getLockOnPos;

	// 既定値(追尾なしのアステロイド)で作る
	ShooterConfig(void);

	// アステロイド用の設定を作る(直進する弾)
	static ShooterConfig MakeAsteroid(void);

	// ハウンド用の設定を作る(目標を追尾する弾)
	// getLockOnPos: 目標の座標を返す関数(省略すると、追尾せずにまっすぐ飛ぶ)
	static ShooterConfig MakeHound(const std::function<std::optional<Vector3>(void)>& getLockOnPos = nullptr);
};
