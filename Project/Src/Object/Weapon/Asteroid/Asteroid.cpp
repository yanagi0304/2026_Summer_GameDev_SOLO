#include "Asteroid.h"

#include <algorithm>

#include "../../Common/Collider/BoxCollider.h"
#include "../../../Manager/TimeScale/TimeScale.h"

static int glowGraph = -1;

static int MakeGlowGraph(void)
{
	const int size = 128;
	const int soft = MakeARGB8ColorSoftImage(size, size);

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			// 中心からの距離(0=中心, 1=端)
			const float dx = (x - size * 0.5f) / (size * 0.5f);
			const float dy = (y - size * 0.5f) / (size * 0.5f);
			const float d = (std::min)(1.0f, std::sqrt(dx * dx + dy * dy));

			// 中心が明るく、外側へなめらかに消える
			const float a = std::pow(1.0f - d, 2.0f);

			DrawPixelSoftImage(soft, x, y, 255, 255, 255, static_cast<int>(a * 255));
		}
	}

	const int graph = CreateGraphFromSoftImage(soft);
	DeleteSoftImage(soft);
	return graph;
}

Asteroid::Asteroid(const Transform& ownerTrans, const ShooterConfig& config) :
	WeaponBase(ownerTrans, config.parameterPath),

	state_(STATE::Hold),
	moveDir_(Vector3::Zonly(1.0f)),
	trail_(),
	baseScale(GetParameter("Init", "scale")),
	config(config)
{
}

void Asteroid::Load(void)
{
	static int sourceModel = -1;
	if (sourceModel < 0) {
		// 複製元は非同期読み込み中だと使えないため、ここだけ同期で読む
		const int prevAsync = GetUseASyncLoadFlag();
		SetUseASyncLoadFlag(FALSE);
		sourceModel = MV1LoadModel("Data/Model/Weapon/Asteroid.mv1");
		SetUseASyncLoadFlag(prevAsync);
	}

	if (glowGraph < 0) { glowGraph = MakeGlowGraph(); }

	trans.Duplicate(sourceModel);

	AddCollider(new BoxCollider(COLLIDER_TAG::Asteroid, GetParameterToVector3("Collider", "Size"), Vector3(0.0f, 0.0f, 0.0f)));

	SetIsDraw(false);
	SetJudgeFlg(false);
	SetContactResponseEnabled(true);

	EnablePhysics();

}

void Asteroid::Appear(void)
{
	// 更新・描画・当たり判定をオンにする
	SetIsActive(true);

	// 待機状態に戻す
	Recall();
}

void Asteroid::Disappear(void)
{
	// 更新・描画・当たり判定をオフにする
	SetIsActive(false);

	ResetMotion();
}

void Asteroid::Place(const Vector3& pos, const Vector3& scale)
{
	trans.pos = pos;
	trans.scale = scale;
}

void Asteroid::Fire(const Vector3& dir)
{
	// 向きが0に近いときは、いまの向きのまま飛ばす
	if (dir.LengthSq() > 1.0e-6f) { moveDir_ = dir.Normalized(); }

	trail_.clear();

	state_ = STATE::Flying;

	// 飛んでいる間だけ当たり判定を有効にする
	SetJudgeFlg(true);
}

void Asteroid::Recall(void)
{
	ResetMotion();

	// 命中して消えていた弾も、ここで見えるように戻す
	SetIsDraw(true);

	// 手元にいる間は当たり判定を使わない
	SetJudgeFlg(false);
}

void Asteroid::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	// 飛んでいる間だけ反応する
	if (state_ != STATE::Flying) { return; }

	const COLLIDER_TAG tag = other.GetTag();

	// ステージか盾以外には反応しない
	if (tag != COLLIDER_TAG::Stage && tag != COLLIDER_TAG::Shield && tag != COLLIDER_TAG::Mannequin) { return; }

	if (onHit) { onHit(result.point, result.normal); }

	Kill();
}

void Asteroid::SubInit(void)
{
	trans.scale = Vector3(baseScale);
}

void Asteroid::SubUpdate(void)
{
	// 飛んでいるとき以外は何もしない
	if (state_ != STATE::Flying) { return; }

	// 軌跡を記録する
	trail_.push_front(trans.pos);
	if (static_cast<int>(trail_.size()) > TRAIL_MAX) { trail_.pop_back(); }

	// 追尾弾なら、ロックオン中の目標へ向きを寄せる
	if (config.getLockOnPos) {

		const std::optional<Vector3> lockOnPos = config.getLockOnPos();

		if (lockOnPos.has_value()) { Steer(lockOnPos.value()); }
	}

	// 進行方向へ加速させる
	PhysicsBody* body = GetPhysicsBody();
	if (body != nullptr) { body->AddAcceleration(moveDir_ * FIRE_ACCEL); }
}

void Asteroid::SubDraw(void)
{
	//// ---- グロー ----
	//SetWriteZBufferFlag(FALSE);                  // 深度は書かない
	//SetDrawBlendMode(DX_BLENDMODE_ADD, 255);     // 加算(強さ 0〜255)
	//SetDrawBright(150, 255, 210);                // ミントグリーンに着色

	//const float size = 80.0f;     // 立方体より大きく(倍率は調整)
	//DrawBillboard3D(trans.pos.ToVECTOR(), 0.5f, 0.5f, size, 0.0f, glowGraph, TRUE);

	//SetDrawBright(255, 255, 255);                // 元に戻す
	//SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	//SetWriteZBufferFlag(TRUE);

	if (trail_.size() < 2) { return; }

	// 加算合成
	SetWriteZBufferFlag(false);

	for (size_t i = 0; i + 1 < trail_.size(); i++)
	{
		// 古い軌跡ほど薄くする
		const float rate = 1.0f - static_cast<float>(i) / TRAIL_MAX;
		const int alpha = static_cast<int>(255 * rate);

		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine3D(trail_[i].ToVECTOR(), trail_[i + 1].ToVECTOR(), GetColor(120, 255, 220));
	}

	// 描画設定を元に戻す
	SetWriteZBufferFlag(TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Asteroid::SubRelease(void)
{
}

void Asteroid::ResetMotion(void)
{
	state_ = STATE::Hold;

	trail_.clear();

	// 発射時の速度が残ってズレるのを防ぐ
	if (GetPhysicsBody() != nullptr) { GetPhysicsBody()->SetVelocity(Vector3(0.0f)); }
}

void Asteroid::Kill(void)
{
	// 見た目・当たり判定・動きをすべて止める
	SetIsDraw(false);
	SetJudgeFlg(false);

	ResetMotion();

	state_ = STATE::Dead;
}

void Asteroid::Steer(const Vector3& targetPos)
{
	PhysicsBody* body = GetPhysicsBody();
	if (body == nullptr) { return; }

	// 目標への方向
	const Vector3 toTarget = targetPos - trans.pos;
	if (toTarget.LengthSq() < 1.0e-6f) { return; }

	// 進行方向を、目標の方向へ少しだけ寄せる
	const float rate = (std::min)(1.0f, HOMING_TURN_RATE * TimeScale::Get());
	moveDir_ = (moveDir_ + (toTarget.Normalized() - moveDir_) * rate).Normalized();

	// 速度の向きも合わせる
	const float speed = body->GetVelocity().Length();
	body->SetVelocity(moveDir_ * speed);
}
