#pragma once

#include "../../Common/CharacterBase/CharacterStateBase.h"

class GroundMotionController;

// グラスホッパーで打ち上げられている間の状態(空中の入力移動は不可)
class PlayerGrasshopperJumpState : public CharacterStateBase
{
public:

	PlayerGrasshopperJumpState(
		GroundMotionController& motion,

		const std::function<void(void)>& playAnimeJumpStart,
		const std::function<void(void)>& playAnimeJumpLoop,
		const std::function<void(void)>& playAnimeStamp,

		const std::function<bool(void)>& isAnimeEnd,

		const std::function<float(void)>& getVelocityY,

		const std::function<void(void)>& changeStateIdle,
		const std::function<void(void)>& changeStateJump
	);

	~PlayerGrasshopperJumpState() override = default;

	// 状態遷移後1度行う初期処理
	void Enter(void) override;
	// 更新処理
	void Update(void) override;

	// 状態に入る前に設定する: trueなら打ち上げ直後から移動できる(真上への打ち上げ用)
	void SetControllableFromStart(bool enabled) { controllableFromStart = enabled; }

private:

	// 打ち上げ直後は接地判定が残っていることがあるため、離陸を待つ猶予(フレーム)
	static constexpr float GROUND_GRACE = 10.0f;

	// 上昇中とみなす速度(これを超えたら一度は上昇した扱い)
	static constexpr float RISING_VELOCITY = 10.0f;

	// 地面の移動を制御
	GroundMotionController& motion;

	const std::function<void(void)> playAnimeJumpStart;
	const std::function<void(void)> playAnimeJumpLoop;
	const std::function<void(void)> playAnimeStamp;
	const std::function<bool(void)> isAnimeEnd;
	// プレイヤーの上下方向の速度を取得する関数
	const std::function<float(void)> getVelocityY;
	const std::function<void(void)> changeStateIdle;
	const std::function<void(void)> changeStateJump;

	enum class STEP {
		// 滞空
		Loop,
		// 着地
		Stamp,
	};

	STEP step;

	// 状態に入ってからの経過時間
	float airTime;

	// 一度でも接地が外れたか
	bool hasLeftGround;

	// 打ち上げ直後から移動できるか
	bool controllableFromStart;

	// 一度でも上昇したか(頂点に達したかの判定に使う)
	bool hasRisen;

	// 滞空ループのアニメーションに切り替え済みか
	bool isLoopAnime;
};