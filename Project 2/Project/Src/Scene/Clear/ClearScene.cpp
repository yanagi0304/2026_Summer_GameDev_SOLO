#include "ClearScene.h"

#include"../../Utility/Utility.h"

#include"../../Application/Application.h"

#include"../SceneManager/SceneManager.h"
#include"../../Manager/Input/KeyManager.h"
#include"../../Manager/Camera/Camera.h"

ClearScene::ClearScene()
{
}

void ClearScene::Load(void)
{
}

void ClearScene::Init(void)
{
	KEY::GetIns().SetMouceFixed(false);
	Camera::GetIns().ChangeModeFixedPoint(Vector3(), Vector3());
}

void ClearScene::Update(void)
{
	if (KEY::GetIns().GetInfo(KEY_TYPE::ENTER).down ||
		KEY::GetIns().GetInfo(KEY_TYPE::PAUSE).down) {
		SceneManager::GetIns().ChangeScene(SCENE_ID::TITLE);
		return;
	}
}

void ClearScene::Draw(void)
{
	int x = Application::SCREEN_SIZE_X;
	int y = Application::SCREEN_SIZE_Y;

	DrawString(0, 0, "クリアシーン", 0xffffff);
}

void ClearScene::Release(void)
{
}