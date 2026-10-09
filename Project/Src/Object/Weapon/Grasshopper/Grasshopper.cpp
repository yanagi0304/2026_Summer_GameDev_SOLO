#include "Grasshopper.h"

#include "../../../Utility/Utility.h"

#include "../../../Manager/Input/InputManager.h"

#include "../../Common/Collider/BoxCollider.h"

Grasshopper::Grasshopper(const Transform& ownerTrans,Vector3& velocity)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Grasshopper/"),
	playerVelocity_(velocity),
	tiltPitch_(0),
	tiltRoll_(0)
{
}

void Grasshopper::Load(void)
{
	trans.LoadModel("Weapon/Grasshopper");
	SetJudgeFlg(false);
	SetDynamicFlg(true);
	SetPushFlg(false);

	AddCollider(new BoxCollider(COLLIDER_TAG::Grasshopper, GetParameterToVector3("Collider", "Size")));
	
}

void Grasshopper::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	playerVelocity_ = 40.0f;
	SetJudgeFlg(false);
}

void Grasshopper::Create(void)
{
	SetIsDraw(true);
	SetJudgeFlg(true);
	SetDynamicFlg(true);
	MV1SetOpacityRate(trans.model, 0.4f);

	// オーナーの回転に合わせてローカル位置を回転
	trans.pos =
		ownerTrans_.pos +
		ownerTrans_.VTrans(
			Vector3::YZonly(1.0f, 0.0f)
		);

	auto vec = InputVec() * 40.0f;
	// オーナーの姿勢を基準に、ローカルX軸へ-40度回転
	// 前後入力 → X軸回転、横入力 → Z軸回転
	const Quaternion tiltX = Quaternion::FromRotationX(Deg2Rad(vec.z));
	const Quaternion tiltZ = Quaternion::FromRotationZ(Deg2Rad(-vec.x));

	// ローカル回転(2軸ぶん)を合成
	const Quaternion localRotation = tiltZ * tiltX;

	// オーナーの回転 + Grasshopper固有の回転
	const MATRIX rotationMat =
		MMult(
			localRotation.ToMatrix(),
			ownerTrans_.RotationMat()
		);

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