#include"TitleScene.h"

#include<DxLib.h>
#include"../../Utility/Utility.h"

#include"../../Application/Application.h"

#include"../../Manager/Input/KeyManager.h"
#include"../../Manager/Camera/Camera.h"
#include"../../Manager/Sound/SoundManager.h"
#include"../../Manager/Font/FontManager.h"

#include"../SceneManager/SceneManager.h"

#include"End/EndScene.h"

TitleScene::TitleScene()
{
}

void TitleScene::Load(void)
{
	// 音声のシーン切り替え
	Snd::GetIns().ChangeScene("Title");

	Key::GetIns().SetMouceFixed(false);
}
void TitleScene::Init(void)
{
	// カメラの初期化
	Camera::GetIns().ChangeModeFixedPoint(Vector3(), Vector3());
}
void TitleScene::Update(void)
{
	// ゲーム終了処理
	if (Key::GetIns().GetInfo(KEY_TYPE::PAUSE).down) {
		Snd::GetIns().Pause();
 		Snd::GetIns().Play("SystemSelect");
		SceneManager::GetIns().PushScene(std::make_shared<EndScene>());
		return;
	}

	// シーン進行処理
	if (Key::GetIns().GetInfo(KEY_TYPE::ENTER).down) {
		Snd::GetIns().Play("SystemButton");
		SceneManager::GetIns().ChangeSceneFade(SCENE_ID::GAME);
		return;
	}
}
void TitleScene::Draw(void)
{
	DrawFormatStringToHandle(0, 0, 0xffffff, Font::GetIns().GetFont(FontKinds::DEFAULT_64), "タイトルシーン");
}
void TitleScene::Release(void)
{
}
