#include "EnemyIdleState.h"

EnemyIdleState::EnemyIdleState(
	std::function<bool(void)> shouldChase,
	std::function<void(void)> playAnimeIdle
) :
	shouldChase(shouldChase),
	playAnimeIdle(playAnimeIdle)
{
}

void EnemyIdleState::OwnStateConditionUpdate(void)
{
	// ’ÇÕ‚·‚é——R‚ª‚È‚­‚È‚Á‚½‚ç‘Ò‹@‚É–ß‚é
	if (!shouldChase()) { OwnChangeState(); }
}

void EnemyIdleState::Enter(void)
{
	playAnimeIdle();
}
