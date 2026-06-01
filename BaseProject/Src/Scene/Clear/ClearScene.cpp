#include "ClearScene.h"

#include"../../Utility/Utility.h"

#include"../../Application/Application.h"

#include"../../Manager/Input/KeyManager.h"
#include"../../Manager/Sound/SoundManager.h"
#include"../../Manager/Font/FontManager.h"

#include"../SceneManager/SceneManager.h"

ClearScene::ClearScene()
{
}

ClearScene::~ClearScene()
{
}

void ClearScene::Load(void)
{
	Snd::GetIns().ChangeScene("Clear");

	Key::GetIns().SetMouceFixed(false);
}

void ClearScene::Init(void)
{
}

void ClearScene::Update(void)
{
	if (Key::GetIns().GetInfo(KEY_TYPE::ENTER).down ||
		Key::GetIns().GetInfo(KEY_TYPE::PAUSE).down) {
		SceneManager::GetIns().ChangeSceneFade(SCENE_ID::TITLE);
		Snd::GetIns().Play("SystemButton");
		return;
	}
}

void ClearScene::Draw(void)
{
	DrawFormatStringToHandle(0, 0, 0xffffff, Font::GetIns().GetFont(FontKinds::DEFAULT_64), "クリアシーン");
}

void ClearScene::Release(void)
{
}