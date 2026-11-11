#include "Mannequin.h"

#include "../../pch.h"

#include "../Common/Collider/CapsuleCollider.h"

#include "../Weapon/Kogetsu/Kogetsu.h"

#include "../Weapon/Manager/ShooterManager.h"

#include "../../Manager/Camera/CurrentCamera.h"

Mannequin::Mannequin(const Vector3& pos, bool AttackFlg, const Vector3& targetPos)
	:CuttableActor(COLLIDER_TAG::DebugObject, true, "Data/Parameter/Mannequin/"),
	targetPos(targetPos)
{
	//位置情報を設定
	trans.pos = pos;

}

void Mannequin::Load(void)
{

	// PhysicsBodyの生成
	EnablePhysics();

	// 切断用のMeshColliderを生成して登録する
	CuttableActor::Load();

	trans.LoadModel("Mannequin/mannequin");

	// モデルの形とテクスチャをCuttableActorへ取り込む
	// （位置・向き・大きさはこのアクターのtransで決まる）
	SetMeshFromModel(trans.model);

	// 元のモデルは描画しない（テクスチャを借りているので、削除はしない）
	MV1SetVisible(trans.model, FALSE);

	// 当たり判定による押し出しを有効にする
	SetContactResponseEnabled(true);
	// 弾性力の設定
	GetPhysicsBody()->SetRestitution(0.0f);
	// 重力を有効にする
	GetPhysicsBody()->SetGravityEnabled(false);
	//マネキン用の物理挙動の設定
	GetPhysicsBody()->SetMode(PhysicsBody::MotionMode::Kinematic);
	// 静的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Static);

#pragma region 武器の生成
	// 「アステロイド」生成

	weapon = new ShooterManager(
		trans,
		ShooterConfig::MakeHound([this]() -> std::optional<Vector3> {return targetPos+Vector3::Yonly(200.0f); }),
		[&]() {std::nullopt; });

	AddChildActor(weapon);
#pragma endregion

}

void Mannequin::SubInit(void)
{
	trans.scale = GetParameter("Init", "Scale");
	weapon->SetIsActive(true);
}

void Mannequin::SubUpdate(void)
{

	// 消えたら処理を止める
	//子アクターの処理も止める
	if (IsVanished()) { weapon->SetIsActive(false); return; }

	// 切断された破片としての処理を先に行う
	CuttableActor::SubUpdate();

	//ターゲットの方向を向く
	TargetLookAt();
}

void Mannequin::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	if (other.GetTag() != COLLIDER_TAG::Asteroid) { return; }
	//StartVanish();
}

void Mannequin::TargetLookAt(void)
{
	// 向く先がないときは、向きを変えない
	const std::optional<Vector3> lookAtPos = targetPos;
	if (!lookAtPos.has_value()) { return; }

	// 向く先への水平方向(上下は無視する)
	Vector3 toTarget = lookAtPos.value() - trans.pos;
	toTarget.y = 0.0f;
	if (toTarget.LengthSq() < 1.0e-6f) { return; }

	// 向く先を向く回転
	const float yaw = std::atan2(toTarget.x, toTarget.z) + MODEL_FORWARD_OFFSET;
	const Quaternion targetRotation = Quaternion::FromRotationY(yaw);

	// 少しずつ向きを寄せる
	const float rate = (std::min)(1.0f, TURN_RATE * TimeScale::Get());
	trans.SetRotation(Quaternion::Slerp(trans.rotation, targetRotation, rate));
}
