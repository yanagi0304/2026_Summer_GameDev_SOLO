#include "PlayerGrasshopperJumpState.h"

#include "../../../Common/Vector3.h"

#include "../../../Manager/Input/InputManager.h"
#include "../../../Manager/Camera/CurrentCamera.h"
#include "../../../Manager/TimeScale/TimeScale.h"

#include "../../Common/MotionController/GroundMotionController.h"
#include "../../../Utility/Utility.h"

PlayerGrasshopperJumpState::PlayerGrasshopperJumpState(
	GroundMotionController& motion,

	const std::function<void(void)>& playAnimeJumpStart,
	const std::function<void(void)>& playAnimeJumpLoop,
	const std::function<void(void)>& playAnimeStamp,

	const std::function<bool(void)>& isAnimeEnd,

	const std::function<float(void)>& getVelocityY,

	const std::function<void(void)>& changeStateIdle,
	const std::function<void(void)>& changeStateJump
) :
	motion(motion),

	playAnimeJumpStart(playAnimeJumpStart),
	playAnimeJumpLoop(playAnimeJumpLoop),
	playAnimeStamp(playAnimeStamp),
	isAnimeEnd(isAnimeEnd),
	getVelocityY(getVelocityY),

	changeStateIdle(changeStateIdle),
	changeStateJump(changeStateJump),

	step(STEP::Loop),
	airTime(0.0f),
	hasLeftGround(false),
	controllableFromStart(false),
	hasRisen(false),
	isLoopAnime(false)
{
}

void PlayerGrasshopperJumpState::Enter(void)
{
	step = STEP::Loop;
	airTime = 0.0f;
	hasLeftGround = false;
	hasRisen = false;
	isLoopAnime = false;

	// ジャンプ開始アニメーションが終わったら滞空アニメーションへ
	if (!isLoopAnime) {
		playAnimeJumpLoop();
		isLoopAnime = true;
	}
}

void PlayerGrasshopperJumpState::Update(void)
{
	switch (step) {

	case STEP::Loop: {

		

		// コントローラーの入力を取得
		Vector3 vec = Input::GetIns().GetLeftStickVec(false).ToVector3XZInvertY();
		// 入力がなければ次にキーボードの入力を取得
		if (vec == 0.0f) {
			if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveRight).now) { vec.x++; }
			if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveLeft).now) { vec.x--; }
			if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveFront).now) { vec.z++; }
			if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveBack).now) { vec.z--; }
			vec.Normalize();
		}

		airTime += TimeScale::Get();

		// 上昇が終わって頂点に達したら(Y方向の速度がなくなったら)移動できる
		const float velY = getVelocityY();
		if (velY > RISING_VELOCITY) { hasRisen = true; }
		const bool canMove = controllableFromStart || ((hasRisen || airTime >= GROUND_GRACE) && velY <= 0.0f);

		vec.TransMatOwn(MGetRotY(CurrentCamera::Get().GetRotation().GetYaw()));
		if (canMove && vec != 0.0f) {
			// 移動
			motion.ApplyInput(vec);
		}
		else {
			// 上昇中は向きだけ変えられる
			motion.FaceDirection(vec);
		}

		if (!motion.IsGrounded()) { hasLeftGround = true; }

		// 着地
		if (motion.IsGrounded() && (hasLeftGround || airTime >= GROUND_GRACE)) {
			playAnimeStamp();
			step = STEP::Stamp;
		}

		break;
	}

	case STEP::Stamp: {

		// 着地中でもジャンプキーで通常ジャンプに移れる
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerJump).down) {
			changeStateJump();
			return;
		}

		// アニメーション終了で待機状態に戻る
		if (isAnimeEnd()) { changeStateIdle(); }

		break;
	}

	}
}