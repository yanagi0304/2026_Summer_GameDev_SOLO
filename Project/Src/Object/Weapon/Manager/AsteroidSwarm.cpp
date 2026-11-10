#include "AsteroidSwarm.h"

#include "../Asteroid/Asteroid.h"

AsteroidSwarm::AsteroidSwarm(void) :
	bullets_(),
	offsets_()
{
	bullets_.reserve(BULLET_NUM);
	offsets_.reserve(BULLET_NUM);

	// 中心が原点になるように、-1, 0, +1 の格子座標を作る(毎フレーム作らず、ここで1回だけ)
	const float half = (SIDE_NUM - 1) * 0.5f;

	for (int z = 0; z < SIDE_NUM; z++)
	{
		for (int y = 0; y < SIDE_NUM; y++)
		{
			for (int x = 0; x < SIDE_NUM; x++)
			{
				offsets_.emplace_back(x - half, half - y, z - half);
			}
		}
	}
}

void AsteroidSwarm::Add(Asteroid* bullet)
{
	if (bullet != nullptr) { bullets_.push_back(bullet); }
}

void AsteroidSwarm::HideAll(void)
{
	for (Asteroid* bullet : bullets_) { bullet->Disappear(); }
}

void AsteroidSwarm::ShowOne(const Vector3& center)
{
	for (size_t i = 0; i < bullets_.size(); i++)
	{
		// 先頭の1発だけ出して、残りは消す
		if (i == 0) { bullets_[i]->Appear(); }
		else { bullets_[i]->Disappear(); }
	}

	FollowOne(center);
}

void AsteroidSwarm::FollowOne(const Vector3& center)
{
	if (bullets_.empty()) { return; }

	// 元の大きさで、手元の位置へ置く
	Asteroid* first = bullets_.front();
	first->Place(center, Vector3(first->GetBaseScale()));
}

void AsteroidSwarm::ShowFormation(const Transform& ownerTrans, const Vector3& center)
{
	for (Asteroid* bullet : bullets_) { bullet->Appear(); }

	FollowFormation(ownerTrans, center);
}

void AsteroidSwarm::FollowFormation(const Transform& ownerTrans, const Vector3& center)
{
	// 持ち主の前方向・右方向
	const Vector3 forward = ownerTrans.VTrans(Vector3(0.0f, 0.0f, 1.0f));
	const Vector3 right = ownerTrans.VTrans(Vector3(1.0f, 0.0f, 0.0f));
	const Vector3 up(0.0f, 1.0f, 0.0f);

	for (size_t i = 0; i < bullets_.size() && i < offsets_.size(); i++)
	{
		const Vector3& offset = offsets_[i];

		const Vector3 pos =
			center +
			right * (offset.x * SPACE) +
			up * (offset.y * SPACE) +
			forward * (offset.z * SPACE);

		bullets_[i]->Place(pos, Vector3(SPLIT_SCALE));
	}
}

void AsteroidSwarm::FireAll(const Vector3& dir)
{
	for (Asteroid* bullet : bullets_) { bullet->Fire(dir); }
}

void AsteroidSwarm::RecallAll(void)
{
	for (Asteroid* bullet : bullets_) { bullet->Recall(); }
}
