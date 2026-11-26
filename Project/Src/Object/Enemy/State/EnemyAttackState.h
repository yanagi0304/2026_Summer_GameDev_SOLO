#pragma once

#include <functional>

#include "../../Common/CharacterBase/CharacterStateBase.h"

#include "../../../Common/Vector3.h"

class GroundMotionController;

// 敵の近接攻撃状態(弧月を振る)
//   Startup : 振りかぶり。目標の方を向く
//   Active  : 攻撃判定(Characterの前方に出すコライダー)が出ている
//   Recovery: 振り終わり。アニメーションが終わったら onFinished を呼ぶ
// 攻撃の判定は、アニメーションの再生割合で出し入れする(プレイヤーの弧月の攻撃と同じ作り)
class EnemyAttackState : public CharacterStateBase
{
public:

	// 攻撃の設定
	struct Params
	{
		// 攻撃判定を出す / 消す、アニメーションの再生割合(0?1)
		// プレイヤーの弧月(1振り目)と同じタイミング
		float activeStartRatio = 0.3f;
		float activeEndRatio = 0.7f;
	};

	// 状態が使う、Enemyの機能
	struct Callbacks
	{
		// 攻撃を始めてよいか(射程内・目標が見えている・クールダウン明け)
		std::function<bool(void)> canStart;

		// 目標の方向(向きを合わせる用。水平成分だけ使う)
		std::function<Vector3(void)> getTargetDirection;

		std::function<void(void)> playAnimeAttack;
		std::function<float(void)> getAnimeRatio;
		std::function<bool(void)> isAnimeEnd;

		std::function<void(void)> hitboxOn;
		std::function<void(void)> hitboxOff;

		// 攻撃が終わった時(次の状態への遷移は、Enemy側で決める)
		std::function<void(void)> onFinished;
	};

	EnemyAttackState(const Params& params, GroundMotionController& motion, const Callbacks& callbacks);

	// 攻撃できる状況なら、この状態へ遷移する
	void OwnStateConditionUpdate(void)override;

	void Enter(void)override;
	void Update(void)override;
	void Exit(void)override;

private:

	enum class PHASE { Startup, Active, Recovery };

	const Params params;
	GroundMotionController& motion;
	const Callbacks callbacks;

	PHASE phase;

	// この状態に入ってからのフレーム数(入った直後は、前のアニメーションの値が見えることがあるので、終了判定を少し待つ)
	int frames;
};
