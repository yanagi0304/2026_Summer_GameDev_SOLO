#include "Kogetsu.h"

Kogetsu::Kogetsu()
	:
	WeaponBase("Data/Parameter/Weapon/")
{

}

void Kogetsu::WeaponLoad(void)
{
	trans.Load("Weapon/Kogetsu");
	WeaponInit();
}

void Kogetsu::WeaponInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = GetParameterToVector3("Kogetsu", "scale");
}

void Kogetsu::WeaponUpdate(void)
{
}

void Kogetsu::WeaponDraw(void)
{
}

void Kogetsu::WeaponAlphaDraw(void)
{
}

void Kogetsu::WeaponUiDraw(void)
{
}

void Kogetsu::WeaponRelease(void)
{
}
