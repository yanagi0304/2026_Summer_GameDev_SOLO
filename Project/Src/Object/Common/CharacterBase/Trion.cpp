#include "Trion.h"
#include "../../../Scene/SceneManager.h"

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
	// 負の値にならないよう0で止める(std::max は windows.h の max マクロと衝突するため使わない)
	if (trionCurrent < 0.0f) { trionCurrent = 0.0f; }
	if (IsDead()) { OnDead(); }
	return true;
}

void Trion::OnDead(void)
{
	//SceneManager::GetIns().ChangeSceneFade(SCENE_ID::GameOver);
}
