#include "EnemyAttackHitbox.h"

#include "../Common/Collider/SphereCollider.h"

namespace
{
	// 単位はcm。プレイヤーのコライダー(半径40)に当たる距離: 持ち主の前方 60?220cm
	constexpr float HIT_RADIUS = 80.0f;

	// 持ち主から見た判定の中心(前方140cm、高さ110cm)。持ち主の本体の回転だけが掛かる
	const Vector3 HIT_OFFSET = Vector3(0.0f, 110.0f, 140.0f);
}

EnemyAttackHitbox::EnemyAttackHitbox(const Transform& ownerTrans, std::function<void(void)> onHitPlayer) :
	ActorBase(),

	ownerTrans(ownerTrans),
	onHitPlayer(onHitPlayer),

	collider(nullptr),
	active(false),
	hitThisSwing(false)
{
}

void EnemyAttackHitbox::Load(void)
{
	// 判定だけのアクター。押し出しはせず、モデルも描画しない
	SetColliderMobility(COLLIDER_MOBILITY::Movable);
	SetContactResponseEnabled(false);
	SetIsDraw(false);

	collider = new SphereCollider(COLLIDER_TAG::EnemyAttack, HIT_RADIUS, HIT_OFFSET);
	collider->SetJudgeFlg(false);
	AddCollider(collider);
}

void EnemyAttackHitbox::On(void)
{
	active = true;
	hitThisSwing = false;
	if (collider != nullptr) { collider->SetJudgeFlg(true); }
}

void EnemyAttackHitbox::Off(void)
{
	active = false;
	if (collider != nullptr) { collider->SetJudgeFlg(false); }
}

void EnemyAttackHitbox::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	if (!active || hitThisSwing) { return; }
	if (other.GetTag() != COLLIDER_TAG::Player) { return; }

	// この一振りでは、これ以上当てない
	hitThisSwing = true;
	if (onHitPlayer) { onHitPlayer(); }
}

void EnemyAttackHitbox::SubUpdate(void)
{
	// 持ち主の位置と向きに追従する(コライダーの相対座標は、この回転を受けて、持ち主の前方に出る)
	trans.pos = ownerTrans.pos;
	trans.SetRotation(ownerTrans.rotation);
}
