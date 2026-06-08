#include "Jazz.h"
#include "../../Common/Collider/SphereCollider.h"
#include "../../../Manager/Input/KeyManager.h"

Jazz::Jazz()
	:
	CharacterBase(
		200,	// HP_MAX
		20,		// ATTACK_POWER
		10,		// DEFENSE_POWER
		5.0f	// SPEED_POWER
	)
{
}



void Jazz::CharacterLoad(void)
{
	trans.Load("Character/Jazz/jazz");
	SetJudge(true);
	SetDynamicFlg(true);
	SetGravityFlg(true);
	ColliderCreate(new SphereCollider(COLLIDER_TAG::PLAYER,100.0f));
}

void Jazz::CharactorInit(void)
{
	trans.pos = Vector3(0.0f, 10.0f, 0.0f);
}

void Jazz::CharactorUpdate(void)
{
}

void Jazz::CharactorDraw(void)
{
}

void Jazz::CharactorAlphaDraw(void)
{
}

void Jazz::CharacterUiDraw(void)
{
}

void Jazz::CharactorRelease(void)
{
}

void Jazz::SubUpdate(void)
{
	static constexpr float MOVE_POWER = 4.0f;
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_FRONT).now) { accelSum.z += MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_BACK).now) { accelSum.z -= MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_LEFT).now) { accelSum.x -= MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_RIGHT).now) { accelSum.x += MOVE_POWER; }
}


