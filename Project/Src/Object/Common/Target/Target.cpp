#include "Target.h"

#include <cmath>

#include "../ActorBase/ActorBase.h"

#include "../../../Manager/Camera/CurrentCamera.h"

Target::Target() :
	candidates(),
	ownerPos({}),
    target(nullptr),
    targetPos(),
    maxDistance(3000.0f),
    minDot(0.966f),
    aimHeightOffset(80.0f)
{
}

void Target::Update(void)
{
	// 毎フレームいったん解除してから探し直す
	target = nullptr;

	// カメラの位置と正面方向
	const Vector3 cameraPos = CurrentCamera::Get().GetPos();
	const Vector3 forward = CalcCameraForward();

	// 最も画面中心に近い候補を探す
	float bestDot = minDot;

	for (ActorBase* actor : candidates) {

		// 安全処理
		if (actor == nullptr) { continue; }

		// 狙う座標(足元から少し上)
		Vector3 aimPos = actor->GetTrans().pos;
		aimPos.y += aimHeightOffset;

		// 自分からの距離で範囲外を除外する
		if ((aimPos - ownerPos.value()).LengthSq() > maxDistance * maxDistance) { continue; }

		// カメラから候補へのベクトル
		const Vector3 toActor = aimPos - cameraPos;

		// 同じ位置にいる場合は方向が決まらないので除外する
		if (toActor.LengthSq() < 0.000001f) { continue; }

		// カメラ正面との内積
		const float dot = forward.Dot(toActor.Normalized());

		// しきい値以下、またはこれまでの最良より遠い場合は除外する
		if (dot <= bestDot) { continue; }

		bestDot = dot;
		target = actor;
		targetPos = aimPos;
	}
}

void Target::Draw(void)
{
	// ターゲットがいなければ何もしない
	if (target == nullptr || !HasTarget()) { return; }
	static int targetMarkHandle_ = LoadGraph("Data/Image/Target/Target_Candidate.png");
	// ターゲット位置(高さ補正つき)
	Vector3 pos = (0,0,0);
	pos.y += 0;

	// カメラの方を向く板を3D空間に描画(Billboard:ビルボード)
	DrawBillboard3D(
		pos.ToVECTOR(),   // 中心位置
		0.5f, 0.5f,       // 画像上の中心(0.5=中央)
		80,        // 大きさ
		0.0f,             // 回転角(ラジアン)
		targetMarkHandle_,
		TRUE              // 透過を有効
	);
}

Vector3 Target::CalcCameraForward(void) const
{
   // const Vector3& angle = CurrentCamera::Get().GetAngle();

 /*   const float pitch = angle.x;
    const float yaw = angle.y;

    return Vector3(
        std::sin(yaw) * std::cos(pitch),
        -std::sin(pitch),
        std::cos(yaw) * std::cos(pitch)
    ).Normalized();*/
	return Vector3();
}
