#include "Grasshopper.h"

#include "../../../Utility/Utility.h"

#include "../../../Manager/Input/InputManager.h"

#include "../../Common/Collider/BoxCollider.h"
#include "../../../Manager/Camera/CurrentCamera.h"

Grasshopper::Grasshopper(const Transform& ownerTrans, bool& isGrashpper)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Grasshopper/"),
	tiltPitch_(0),
	tiltRoll_(0),
	isGrashpper(isGrashpper)
{
}

void Grasshopper::Load(void)
{
	trans.LoadModel("Weapon/Grasshopper");

	AddCollider(new BoxCollider(COLLIDER_TAG::Grasshopper, GetParameterToVector3("Collider", "Size")));

	SetJudgeFlg(false);
	SetDynamicFlg(true);
	SetPushFlg(false);
	SetIsDraw(false);

	SetDrawType(ACTOR_DRAW_TYPE::Alpha);
	
}

void Grasshopper::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{

	if (other.GetTag() != COLLIDER_TAG::Player) { return; }
	//SetJudgeFlg(false);
	isGrashpper = true;

	const VECTOR localUp = VGet(0.0f, 1.0f, 0.0f);
	const Vector3 launchDir = VNorm(VTransformSR(localUp, rotationMat));

	// 飛ばす速度ベクトル(Velocity:ベロシティ=速度)
	const Vector3 launchVelocity = launchDir * 40.0f;
	other.GetActor()->Launch(launchVelocity);
}

void Grasshopper::OnEquip(void)
{
	SetJudgeFlg(false);
	SetDynamicFlg(true);
	SetPushFlg(false);
	SetIsDraw(false);
}

void Grasshopper::Create(void)
{
	SetIsDraw(true);
	SetJudgeFlg(true);
	SetDynamicFlg(true);
	MV1SetOpacityRate(trans.model, 0.4f);

	// オーナーの回転に合わせてローカル位置を回転
	trans.pos =
		ownerTrans.pos +
		ownerTrans.VTrans(
			Vector3::YZonly(1.0f, 0.0f)
		);
	static constexpr float TILT_MAX_DEG = 40.0f;
	// 入力ベクトル
	auto input = InputVec();

	//input.TransMatOwn(MGetRotY(CurrentCamera::Get().GetAngle().y));

	const Quaternion tiltX = Quaternion::FromRotationX(Deg2Rad(input.z * TILT_MAX_DEG));
	const Quaternion tiltZ = Quaternion::FromRotationZ(Deg2Rad(-input.x * TILT_MAX_DEG));

	// ローカル回転を合成
	const Quaternion localRotation = tiltZ * tiltX;
	rotationMat = localRotation.ToMatrix();

	trans.SetRotation(
		Quaternion::FromMatrix(rotationMat)
	);
}

void Grasshopper::SubInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));
	SetIsDraw(false);
	SetJudgeFlg(false);
	SetDynamicFlg(false);
	SetPushFlg(false);
}

void Grasshopper::SubUpdate(void)
{
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down)
	{
		Create();
	}

}

void Grasshopper::SubDraw(void)
{
}

void Grasshopper::SubRelease(void)
{
}

Vector3 Grasshopper::InputVec(void) const
{
	// 返却用一時変数
	Vector3 vec = Vector3();

	// コントローラーの入力を取得
	vec = Input::GetIns().GetLeftStickVec(false).ToVector3XZInvertY();

	// 入力がなければ次にキーボードの入力を取得
	if (vec == 0.0f) {
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveRight).now) { vec.x++; }
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveLeft).now) { vec.x--; }
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveFront).now) { vec.z++; }
		if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveBack).now) { vec.z--; }
		vec.Normalize();
	}

	return vec;
}