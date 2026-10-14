#include "PlayerKogetsuState.h"

#include "../../../Manager/Input/InputManager.h"

#include "../../Common/Transform/Transform.h"




PlayerKogetsuState::PlayerKogetsuState(
	float COLL_START_TIME,
	float COLL_END_TIME,

	const TriggerType triggerType,

	WeaponBase& kogetsu,

	std::function<void(void)> playAnimeSlashA1,
	std::function<void(void)> playAnimeSlashA2,
	std::function<void(void)> playAnimeSlashA3,

	std::function<float(void)> getAnimeRatio,

	std::function<void(void)> changeStateIdle
) :
	COLL_START_TIME(COLL_START_TIME),
	COLL_END_TIME(COLL_END_TIME),

	triggerType(triggerType),

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

	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) {
		OwnChangeState();
	}
}

void PlayerKogetsuState::Enter(void)
{
	// ステップを「前隙」へ
	step = STEP::Startup;

	// 当たり判定を消去
	//collOperator.Off();

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
		if (COLL_START_TIME <= animeRatio) {

			// ステップを「攻撃判定発生中」へ
			step = STEP::Active;

			// 当たり判定を発生
			//collOperator.On();
		}

		break;
	}

	case PlayerKogetsuState::STEP::Active: {
		// 攻撃判定発生中
		// 当たり判定を追従

		//次コンボ猶予受付
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) { NextCombo(); }

		// 攻撃判定発生終了
		if (COLL_END_TIME <= animeRatio) {

			// ステップを「後隙」へ
			step = STEP::Recovery;

			// 当たり判定を消去
			//collOperator.Off();
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
	//collOperator.Off();
}

void PlayerKogetsuState::NextCombo(void)
{
	switch (combo)
	{
	case PlayerKogetsuState::COMBO_STEP::A1:
		playAnimeSlashA2();
		//次コンボに切り替え
		combo = COMBO_STEP::A2;
		break;
	case PlayerKogetsuState::COMBO_STEP::A2:
		playAnimeSlashA3();
		combo = COMBO_STEP::A3;
		break;
	case PlayerKogetsuState::COMBO_STEP::A3:
		break;
	default:
		break;
	}
}
