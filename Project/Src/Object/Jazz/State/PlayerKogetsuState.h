#pragma once

#include "../../Common/CharacterBase/CharacterStateBase.h"

#include "../../../Common/Vector3.h"

struct Transform;

class Kogetsu;

class PlayerKogetsuState : public CharacterStateBase
{
public:

	PlayerKogetsuState(
		float COLL_START_TIME,
		float COLL_END_TIME,

		Kogetsu& collOperator,

		std::function<bool(void)> playAnimeSlashA1,
		std::function<void(void)> playAnimeSlashA2,
		std::function<void(void)> playAnimeSlashA3,

		std::function<float(void)> getAnimeRatio,

		std::function<void(void)> changeStateIdle
	);

	~PlayerKogetsuState()override = default;

	// 自分の状態に遷移する条件関数
	void OwnStateConditionUpdate(void);

	// 状態遷移後1度行う初期化処理
	void Enter(void)override;
	// 更新処理
	void Update(void)override;
	// 状態遷移前1度行う終了処理
	void Exit(void)override;

private:

#pragma region 定数

	// 攻撃ステップ
	enum class STEP
	{
		// 前隙
		Startup,

		// 攻撃判定発生中
		Active,

		// 後隙
		Recovery
	};

	// 攻撃の判定を発生させ始めるアニメーション再生割合
	const float COLL_START_TIME;
	// 攻撃の判定を発生させ終わるアニメーション再生割合
	const float COLL_END_TIME;

#pragma endregion

#pragma region 受け取る参照変数・関数

	// 攻撃の当たり判定管理クラスの参照
	Kogetsu& collOperator;

	//プレイヤーの所持能力を確認する関数のポインタ
	const std::function<bool(void)> playerAbility;

	// 攻撃アニメーションの再生関数のポインタ
	const std::function<void(void)> playAnimeAttack;

	// アニメーションの再生割合を取得する関数のポインタ
	const std::function<float(void)> getAnimeRatio;

	// 待機状態に戻る関数のポインタ
	const std::function<void(void)> changeStateIdle;

#pragma endregion

	// 攻撃ステップ
	STEP step;
};

