#include "CharacterBase.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "../../../Application/Application.h"

#include "CharacterStateBase.h"

#include "Trion.h"

#include "../MotionController/MotionControllerBase.h"
#include "../../../Manager/TimeScale/TimeScale.h"
#include "../../../Manager/Collision/CurrentCollisionManager.h"

#include "../AnimationController/AnimationController.h"

CharacterBase::CharacterBase() :
	ActorBase(),

	state(-1),
	stateMap(),

	motionController(nullptr),

	stepClimb(),
	stepClimbEnabled(true),
	stepClimbState(),
	stepLastPos(0.0f),
	stepLastPosValid(false),
	groundedLoose(false),

	anime(nullptr)
{
}

CharacterBase::CharacterBase(const std::string& parameterPath):
	ActorBase(parameterPath),

	state(0),
	stateMap(),

	motionController(nullptr),

	stepClimb(),
	stepClimbEnabled(true),
	stepClimbState(),
	stepLastPos(0.0f),
	stepLastPosValid(false),
	groundedLoose(false),

	anime(nullptr)
{
}

void CharacterBase::BaseUpdate(void)
{
	// ジャンプや移動の加速に使う接地判定を、状態の更新より前に更新する
	UpdateGroundedLoose();

	// ステートの更新
	if (!stateMap.empty()) {
		stateMap.at(state)->OtherStateConditionsUpdate();
		stateMap.at(state)->Update();
		for (std::pair<const int, CharacterStateBase*>& s : stateMap) { s.second->AlwaysUpdate(); }
	}

	// ステートによる入力反映後、PhysicsBody::Stepの前に共通減速を適用する。
	if (motionController != nullptr) {
		motionController->Update(TimeScale::Get());
	}

	// 段差を登る(速度が決まった後、PhysicsBodyのStepで移動する前に行う)
	UpdateStepClimb();

	// アニメーション更新
	if (anime) {
		anime->Update();
		trans.animationOffset = anime->GetPositionCorrection();
	}
}

void CharacterBase::BaseRelease(void)
{
	// ActorBaseがPhysicsBodyを解放する前にControllerを破棄する。
	delete motionController;
	motionController = nullptr;

	// ステート管理用マップの解放
	for (auto& s : stateMap) {
		if (s.second) { delete s.second; s.second = nullptr; }
	}
	stateMap.clear();

	// アニメーションコントローラーの解放（使われていたら）
	if (anime) {
		anime->Release();
		delete anime;
		anime = nullptr;
	}
}

#pragma region ステート管理関数

void CharacterBase::AddState(int stateNum, CharacterStateBase* stateIns)
{
	// 自分の状態に遷移させる関数のポインタをセットする（共通初期設定）
	stateIns->SetOwnChangeStatePtr([this, stateNum]() { ChangeState(stateNum); });

	// 現在自分の状態かどうかを返す関数のポインタをセットする（共通初期設定）
	stateIns->SetIsOwnStatePtr([this, stateNum]() { return state == stateNum; });

	// 格納
	stateMap.emplace(stateNum, stateIns);
}

void CharacterBase::RegisterStateTransition(int beforeState, int afterState)
{
	GetStateIns(beforeState).AddOtherStateCondition([this, afterState]() { GetStateIns(afterState).OwnStateConditionUpdate(); });
}

void CharacterBase::ChangeState(int nextState)
{
	// 遷移前のステートの終了処理を呼び出す
	if (stateMap.contains(state)) { stateMap.at(state)->Exit(); }

	// 遷移
	state = nextState;

	// 遷移後のステートの初期化処理を呼び出す
	if (stateMap.contains(state)) { stateMap.at(state)->Enter(); }
}

CharacterStateBase& CharacterBase::GetStateIns(int state)
{
	auto it = stateMap.find(state);
	if (it != stateMap.end()) { return *(it->second); }
	else { throw std::runtime_error("指定のステートインスタンスが見つかりません"); }
}

#pragma endregion

#pragma region 段差の自動登り

void CharacterBase::UpdateGroundedLoose(void)
{
	groundedLoose = isGround;
	if (groundedLoose) { return; }

	// 接地フラグが立っていなくても、直下の地面がすぐ近くなら、地面にいるとみなす
	// (縁石の角に斜めに押し上げられた時など、地面の近くでフラグが外れてもジャンプできるようにする)
	PhysicsBody* body = GetPhysicsBody();
	if (body == nullptr || body->IsFrozen() || body->GetMode() != PhysicsBody::MotionMode::Character) { return; }
	if (!CurrentCollisionManager::Exists()) { return; }

	// ジャンプの出だしなど、上へ向かっている間は、地面にいるとみなさない(二重にジャンプしないように)
	if (body->GetVelocity().y > 10.0f) { return; }

	RaycastHit hit;
	const float maxDistance = 20.0f + stepClimb.maxGroundClearance + 30.0f;
	if (CurrentCollisionManager::Get().RaycastStage(Vector3(trans.pos.x, trans.pos.y + 20.0f, trans.pos.z), Vector3(0.0f, -1.0f, 0.0f), maxDistance, hit)
		&& (trans.pos.y - hit.point.y) <= stepClimb.maxGroundClearance)
	{
		groundedLoose = true;
	}
}
void CharacterBase::SetStepClimb(bool enabled, float maxHeight, float radius)
{
	stepClimbEnabled = enabled;
	stepClimb.maxHeight = maxHeight;
	stepClimb.radius = radius;
	stepClimbState.Reset();
}

void CharacterBase::UpdateStepClimb(void)
{
	// 前のフレームの位置を取り出して、今の位置を覚える(早期returnする場合も、毎フレーム更新する)
	const Vector3 lastPos = stepLastPos;
	const bool hadLastPos = stepLastPosValid;
	stepLastPos = trans.pos;
	stepLastPosValid = true;

	// 物理が有効なキャラクターだけが対象。地面にいるかは、判定の中で直下のレイで調べる
	// (接地フラグは、縁石に押しつけて歩くと、角に押し上げられて外れるので使わない)
	if (!stepClimbEnabled) { stepClimbDebug.result = StepClimbResult::Disabled; return; }

	PhysicsBody* body = GetPhysicsBody();
	if (body == nullptr || body->IsFrozen() || body->GetMode() != PhysicsBody::MotionMode::Character) { return; }
	if (!CurrentCollisionManager::Exists()) { return; }

	// 入力の向き(移動制御が、直近のApplyInputで受け取ったもの)。入力がなければ、押しつけていないので登らない
	Vector3 input(0.0f);
	if (motionController == nullptr || !motionController->TakeLastInput(input))
	{
		stepClimbState.Reset();
		stepClimbDebug.result = StepClimbResult::NotPressing;
		return;
	}
	input.y = 0.0f;

	// 目標の移動速度と、入力した向きに実際に進めた距離
	const float intendedSpeed = (input.LengthSq() > 0.01f) ? motionController->GetCruiseSpeed() : 0.0f;
	float progress = 0.0f;
	if (hadLastPos && input.LengthSq() > 1.0e-6f)
	{
		const Vector3 moved = trans.pos - lastPos;
		progress = (std::max)(0.0f, moved.x * input.Normalized().x + moved.z * input.Normalized().z);
	}
	else
	{
		// 前のフレームの位置がない(出現直後など)時は、進めているものとして扱う
		progress = intendedSpeed;
	}

	// ジャンプの出だしなど、上へ向かっている間は登らない
	const Vector3 velocity = body->GetVelocity();
	if (velocity.y > 10.0f) { stepClimbDebug.result = StepClimbResult::NotGrounded; return; }

	// ステージだけを対象にしたレイで、前方の障害物と、背の高い壁かを調べる
	CollisionManager& collision = CurrentCollisionManager::Get();
	const float lift = ComputeForcedLift(trans.pos, input, intendedSpeed, progress, stepClimb,
		[&collision](const Vector3& origin, const Vector3& direction, float maxDistance)
		{
			StepRayHit result;
			RaycastHit hit;
			if (collision.RaycastStage(origin, direction, maxDistance, hit))
			{
				result.hit = true;
				result.distance = hit.distance;
				result.point = hit.point;
				result.normal = hit.normal;
			}
			return result;
		}, stepClimbState, &stepClimbDebug);
	if (lift <= 0.0f) { return; }

	// 少し持ち上げる。落下速度は0にして、持ち上げた分がすぐ戻らないようにする
	trans.pos.y += lift;

	// 壁に押しつけている間は、物理が壁へ向かう速度を毎フレーム打ち消している。
	// 持ち上げが終わって進めるようになった時に、止まらず進めるよう、入力の向きへ通常の移動速度を与えておく
	const Vector3 forward = input.Normalized() * (std::max)(intendedSpeed, std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z));
	body->SetVelocity(Vector3(forward.x, (std::max)(velocity.y, 0.0f), forward.z));
}
#pragma endregion
void CharacterBase::AddMotionController(MotionControllerBase* controller)
{
	// 同一ポインタの再登録では何もしない。
	if (controller == motionController) return;

	// Controllerの所有権をCharacterBaseに移す。
	delete motionController;
	motionController = controller;
	if (motionController) motionController->Init(GetPhysicsBody());
}

#pragma region アニメーションコントローラー

void CharacterBase::CreateAnimationController(void) { if (anime == nullptr) anime = new AnimationController(trans.model); }

void CharacterBase::AddInFbxAnimation(int inFbxMaxIndex, float speed, const bool* const loop)
{
	for (int index = 0; index < inFbxMaxIndex; index++) {
		anime->AddInFbx(index, speed, (loop != nullptr) ? loop[index] : true, index);
	}
}

void CharacterBase::AddInFbxAnimation(int inFbxMaxIndex, const float* const speed, const bool* const loop)
{
	for (int index = 0; index < inFbxMaxIndex; index++) {
		anime->AddInFbx(index, speed[index], (loop != nullptr) ? loop[index] : true, index);
	}
}

void CharacterBase::AddAnimation(int index, float speed, bool loop, const char* filePath)
{
	anime->Add(index, speed, loop, filePath);
}

void CharacterBase::AnimePlay(int type, signed char loop)
{
	anime->Play(type, loop);
}

bool CharacterBase::IsAnimeEnd(void) const { return anime->IsAnimEnd(); }

float CharacterBase::GetAnimeRatio(void) const { return anime->GetAnimeRatio(); }

float CharacterBase::GetAnimeTotalTime(void) const { return anime->GetAnimeTotalTime(); }

int CharacterBase::GetAnimePlayType(void)const { return anime->GetAnimePlayType(); }

float CharacterBase::GetAnimeStep(void)const { return anime->GetAnimeStep(); }

void CharacterBase::SetAnimeStep(float step) { anime->SetAnimeStep(step); }

bool CharacterBase::SetPositionCorrectionFrame(const std::string& frameName) { return anime->SetPositionCorrectionFrame(frameName); }

void CharacterBase::SetPositionCorrection(bool enabled) { anime->SetPositionCorrection(enabled); }

#pragma endregion

void CharacterBase::SetTrionMax(float max) { trion->SetMax(max); }
void CharacterBase::DecreaseTrion(const float decrease) { trion->Decrease(decrease); }
float CharacterBase::GetTrionRatio(void) const{ return trion->GetRatio(); }
bool CharacterBase::IsDeadCharacter(void) const{ return trion->IsDead(); }