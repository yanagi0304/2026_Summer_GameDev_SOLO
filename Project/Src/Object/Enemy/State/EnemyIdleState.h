#pragma once

#include "../../Common/CharacterBase/CharacterStateBase.h"

// 敵の待機状態
// 追跡を続ける理由がなくなったら(目標を見失って時間が経ったら)この状態に戻る
class EnemyIdleState : public CharacterStateBase
{
public:

	EnemyIdleState(
		std::function<bool(void)> shouldChase,
		std::function<void(void)> playAnimeIdle
	);

	~EnemyIdleState()override = default;

	// 追跡する理由がなければ、自分の状態に遷移
	void OwnStateConditionUpdate(void)override;

	// 状態遷移後1度行う初期化処理
	void Enter(void)override;

private:

	// 追跡すべきかどうか
	const std::function<bool(void)> shouldChase;

	// 待機アニメーションの再生関数
	const std::function<void(void)> playAnimeIdle;
};
