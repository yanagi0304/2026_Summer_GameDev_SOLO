#include "Hound.h"

Hound::Hound(const Vector3& vec, const DecreaseTrionFunc& decreaseTrion)
	:WeaponBase(Transform(), "Data/Parameter/Weapon/Asteroid/", decreaseTrion),
	vec(vec)
{
}

void Hound::Load(void)
{
}

void Hound::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
}

void Hound::OnEquip(void)
{
}

void Hound::SubInit(void)
{
}

void Hound::SubUpdate(void)
{
}

void Hound::SubDraw(void)
{
}

void Hound::SubRelease(void)
{
}
