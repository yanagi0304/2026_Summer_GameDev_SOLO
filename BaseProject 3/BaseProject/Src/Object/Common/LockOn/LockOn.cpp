#include <limits>
#include "LockOn.h"

LockOn::LockOn()
	: targetImgCandidate_(-1)
{
}

void LockOn::Load(void)
{
	targetImgCandidate_ = LoadGraph("Data/Image/Target/Target_Candidate.png");
}

void LockOn::Update(const Vector3& PlayerPos, Mannequin* mannequin)
{
	// プレイヤーとマネキンの距離を計算して、最も近いマネキンをロックオン対象にする
	float closestDistance = LOCK_ON_DISTANCE;
	Mannequin* closestMannequin = nullptr;
	//for (Mannequin* mannequin : mannequins) {
	//	if (!mannequin) { continue; }
		float distance = (mannequin->GetTrans().pos - PlayerPos).Length();
		if (distance < closestDistance) {
			closestDistance = distance;
			closestMannequin = mannequin;
		}
	
	// ロックオン対象が見つかった場合、その位置を保存する
	targetMannequin = closestMannequin;
}

void LockOn::Draw(void)
{
	if (!targetMannequin) { return; }

	VECTOR targetPos = targetMannequin->GetTrans().pos.ToVECTOR();

	targetPos.y += 350.0f;

	DrawBillboard3D(targetPos, 0.5f, 0.5f,10.0f, 0.0f, targetImgCandidate_,false);

}