#include "Jazz.h"
#include "../../Common/Collider/CapsuleCollider.h"
#include "../../../Manager/Input/KeyManager.h"
#include "../../../Manager/Camera/Camera.h"




Jazz::Jazz()
	:
	CharacterBase(100, 10, 5, 5, "Data/Parameter/Character/Player/Wo-Chamolenium/")
{
}

void Jazz::CharacterLoad(void)
{
	trans.Load("Character/Jazz/jazz");
	SetJudge(true);
	SetDynamicFlg(true);
	SetGravityFlg(true);
	ColliderCreate(new CapsuleCollider(COLLIDER_TAG::PLAYER, GetParameterToVector3("Collider", "ColliderStartPos")
		, GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));
}

void Jazz::CharactorInit(void)
{
	trans.pos = Vector3(0.0f, 0.0f, 0.0f);
	trans.scale = GetParameterToVector3("Init", "Scale");
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

	Vector3 angle = Camera::GetIns().GetAngle();
	float sinY = sinf(angle.y);
	float cosY = cosf(angle.y);

	Vector3 localAngle = Vector3();

	static constexpr float MOVE_POWER = 4.0f;
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_FRONT).now) { localAngle.z += MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_BACK).now) { localAngle.z -= MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_LEFT).now) { localAngle.x -= MOVE_POWER; }
	if (KeyManager::GetIns().GetInfo(KeyManager::KEY_TYPE::PLAYER_MOVE_RIGHT).now) { localAngle.x += MOVE_POWER; }

	if (localAngle != 0.0f)
	{
		trans.angle.y = atan2f(-localAngle.x, -localAngle.z) + angle.y;
		accelSum.x += localAngle.x * cosY + localAngle.z * sinY;
		accelSum.z += localAngle.z * cosY - localAngle.x * sinY;
	}

}


