#include "EnemyAttackState.h"

#include "../../Common/MotionController/GroundMotionController.h"

namespace
{
	// 入ってから、アニメーションの終了を見始めるまでのフレーム数
	constexpr int END_CHECK_DELAY_FRAMES = 2;
}

EnemyAttackState::EnemyAttackState(const Params& params, GroundMotionController& motion, const Callbacks& callbacks) :
	params(params),
	motion(motion),
	callbacks(callbacks),
	phase(PHASE::Startup),
	frames(0)
{
}

void EnemyAttackState::OwnStateConditionUpdate(void)
{
	if (callbacks.canStart && callbacks.canStart()) { OwnChangeState(); }
}

void EnemyAttackState::Enter(void)
{
	phase = PHASE::Startup;
	frames = 0;

	if (callbacks.hitboxOff) { callbacks.hitboxOff(); }
	if (callbacks.playAnimeAttack) { callbacks.playAnimeAttack(); }
}

void EnemyAttackState::Update(void)
{
	++frames;
	const float ratio = callbacks.getAnimeRatio ? callbacks.getAnimeRatio() : 0.0f;

	switch (phase)
	{
	case PHASE::Startup:
	{
		// 振りかぶっている間は、目標の方を向く(移動はしない。向きだけ変える)
		if (callbacks.getTargetDirection)
		{
			Vector3 dir = callbacks.getTargetDirection();
			dir.y = 0.0f;
			if (dir.LengthSq() > 1.0e-6f) { motion.FaceDirection(dir.Normalized()); }
		}

		if (frames >= END_CHECK_DELAY_FRAMES && ratio >= params.activeStartRatio)
		{
			phase = PHASE::Active;
			if (callbacks.hitboxOn) { callbacks.hitboxOn(); }
		}
		break;
	}

	case PHASE::Active:
		if (ratio >= params.activeEndRatio)
		{
			phase = PHASE::Recovery;
			if (callbacks.hitboxOff) { callbacks.hitboxOff(); }
		}
		break;

	case PHASE::Recovery:
		break;
	}

	// アニメーションが終わったら、攻撃も終わり
	if (frames >= END_CHECK_DELAY_FRAMES && callbacks.isAnimeEnd && callbacks.isAnimeEnd())
	{
		if (callbacks.onFinished) { callbacks.onFinished(); }
	}
}

void EnemyAttackState::Exit(void)
{
	// 途中で状態が変わっても、判定が残らないようにする
	if (callbacks.hitboxOff) { callbacks.hitboxOff(); }
}
