#include "PlayerKogetsuState.h"

#include "../../../Manager/Input/InputManager.h"

#include "../../Common/Transform/Transform.h"

#include "../../Weapon/Kogetsu/Kogetsu.h"

PlayerKogetsuState::PlayerKogetsuState(
	const std::array<float, (size_t)COMBO_STEP::Max>& COLL_START_TIME,
	const std::array<float, (size_t)COMBO_STEP::Max>& COLL_END_TIME,

	WeaponBase& kogetsu,

	std::function<bool(void)> isTrigger,

	std::function<void(void)> playAnimeSlashA1,
	std::function<void(void)> playAnimeSlashA2,
	std::function<void(void)> playAnimeSlashA3,

	std::function<float(void)> getAnimeRatio,

	std::function<void(void)> changeStateIdle
) :
	COLL_START_TIME(COLL_START_TIME),
	COLL_END_TIME(COLL_END_TIME),

	isTrigger(isTrigger),

	kogetsu(kogetsu),

	playAnimeSlashA1(playAnimeSlashA1),
	playAnimeSlashA2(playAnimeSlashA2),
	playAnimeSlashA3(playAnimeSlashA3),

	getAnimeRatio(getAnimeRatio),

	changeStateIdle(changeStateIdle),

	step()
{
}


void PlayerKogetsuState::OwnStateConditionUpdate(void)
{

	//所持トリガーが弧月じゃなければ戻す
	if (!isTrigger()) { return; }

	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) {
		OwnChangeState();
	}
}

void PlayerKogetsuState::Enter(void)
{
	// ステップを「前隙」へ
	step = STEP::Startup;

	// 当たり判定を消去
	kogetsu.Off();

	// 攻撃アニメーション再生
	playAnimeSlashA1();

	//コンボを一段階へ
	combo = COMBO_STEP::A1;
}

void PlayerKogetsuState::Update(void)
{
	// アニメーションの再生割合を取得
	const float animeRatio = getAnimeRatio();

	// ステップ別更新
	switch (step) {

	case PlayerKogetsuState::STEP::Startup: {
		// 前隙

		// 攻撃判定発生開始
		if (COLL_START_TIME[(size_t)combo] <= animeRatio) {

			// ステップを「攻撃判定発生中」へ
			step = STEP::Active;

			// 当たり判定を発生
			kogetsu.On();
		}

		break;
	}

	case PlayerKogetsuState::STEP::Active: {
		// 攻撃判定発生中
		// 当たり判定を追従

			//次コンボ猶予受付
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) { isInputBuffered = true; }

		// 攻撃判定発生終了
		if (COLL_END_TIME[(size_t)combo] <= animeRatio) {

			// 入力がなかったらステップを「後隙」へ
			if (!isInputBuffered) { step = STEP::Recovery; }
			else { NextCombo();}

			// 当たり判定を消去
			kogetsu.Off();
		}

		break;
	}

	case PlayerKogetsuState::STEP::Recovery: {
		// 後隙

		// アニメーション再生終了で強制的に待機状態へ
		if (1.0f <= animeRatio) {

			// 待機状態へ遷移
			changeStateIdle();
		}

		break;
	}

	}
}

void PlayerKogetsuState::Exit(void)
{
	// 当たり判定を消去
	kogetsu.Off();
}

void PlayerKogetsuState::NextCombo(void)
{
	switch (combo)
	{
	case PlayerKogetsuState::COMBO_STEP::A1:
		playAnimeSlashA2();
		//次コンボに切り替え
		combo = COMBO_STEP::A2;
		isInputBuffered = false;
		step = STEP::Startup;
		break;
	case PlayerKogetsuState::COMBO_STEP::A2:
		playAnimeSlashA3();
		combo = COMBO_STEP::A3;
		isInputBuffered = false;
		step = STEP::Startup;
		break;
	case PlayerKogetsuState::COMBO_STEP::A3:
		STEP::Recovery;
		break;
	default:
		break;
	}
}
