#pragma once

#include <functional>

#include "../Common/ActorBase/ActorBase.h"

class SphereCollider;

// 敵の攻撃判定(判定だけを持つアクター)
// 持ち主の前方に、球のコライダーを出す。
// 弧月に付いているコライダーは、メッシュカット専用なので、攻撃の判定には使わない。
//
// On()からOff()までの間だけ判定が有効になる。
// 1回の振り(On?Off)で、プレイヤーに当たるのは1回だけ(多段ヒットしない)。
class EnemyAttackHitbox : public ActorBase
{
public:

	/// <param name="ownerTrans">持ち主のTransform(前方に出すために、位置と向きを参照する)</param>
	/// <param name="onHitPlayer">プレイヤーに当たった時に呼ぶ関数</param>
	EnemyAttackHitbox(const Transform& ownerTrans, std::function<void(void)> onHitPlayer);

	~EnemyAttackHitbox()override = default;

	void Load(void)override;

	// 判定を出す(この一振りで、まだ当たっていない状態に戻す)
	void On(void);

	// 判定を消す
	void Off(void);

	bool IsOn(void)const { return active; }

	void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

private:

	// 持ち主の位置と向きに追従する
	void SubUpdate(void)override;

	const Transform& ownerTrans;
	const std::function<void(void)> onHitPlayer;

	SphereCollider* collider;

	// 判定が有効か / この一振りで、すでに当たったか
	bool active;
	bool hitThisSwing;
};
