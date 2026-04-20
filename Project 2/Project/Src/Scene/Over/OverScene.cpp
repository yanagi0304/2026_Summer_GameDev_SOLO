#include"OverScene.h"

#include"../../Utility/Utility.h"

#include"../../Manager/Input/KeyManager.h"

#include"../../Application/Application.h"
#include"../SceneManager/SceneManager.h"

OverScene::OverScene()
{
}

void OverScene::Load(void)
{
}

void OverScene::Init(void)
{
	KEY::GetIns().SetMouceFixed(false);
}

void OverScene::Update(void)
{
	if (KEY::GetIns().GetInfo(KEY_TYPE::ENTER).down ||
		KEY::GetIns().GetInfo(KEY_TYPE::PAUSE).down) {
		SceneManager::GetIns().ChangeScene(SCENE_ID::TITLE);
		return;
	}
}

void OverScene::Draw(void)
{
	DrawString(0, 0, "ゲームオーバーシーン", 0xffffff);
}

void OverScene::Release(void)
{
}