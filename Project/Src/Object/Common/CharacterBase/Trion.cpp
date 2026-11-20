#include "Trion.h"

void Trion::Load(void)
{
	trionMax = 100.0f;
	trionCurrent = trionMax;
}

void Trion::Init(void)
{
	trionCurrent = trionMax;
}

void Trion::Update(void)
{
}

void Trion::Draw(void)
{
}

void Trion::Release(void)
{
}

bool Trion::Decrease(const float decrease)
{
	if (IsDead()) { return false; }
	trionCurrent -= decrease;
	if (IsDead()) { OnDead(); }
	return true;
}

void Trion::OnDead(void)
{
}
