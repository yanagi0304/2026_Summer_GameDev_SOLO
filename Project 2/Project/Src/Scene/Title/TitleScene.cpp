#include"TitleScene.h"

#include<DxLib.h>

#include"../../Manager/Input/KeyManager.h"
#include"../../Manager/Camera/Camera.h"

#include"../../Application/Application.h"
#include"../SceneManager/SceneManager.h"
#include"../../Utility/Utility.h"

#include"End/EndScene.h"

#include"../../Object/SkyDome/SkyDome.h"


TitleScene::TitleScene():
	skyDome_(nullptr)
{
}

void TitleScene::Load(void)
{
	skyDome_ = new SkyDome();
	skyDome_->Load();
	skyDome_->SetPos({0.0f,0.0f,0.0f});

}
void TitleScene::Init(void)
{
	KEY::GetIns().SetMouceFixed(false);
	Camera::GetIns().ChangeModeFixedPoint(Vector3(), Vector3());
}
void TitleScene::Update(void)
{
	if (KEY::GetIns().GetInfo(KEY_TYPE::PAUSE).down) {
		SceneManager::GetIns().PushScene(std::make_shared<EndScene>());
		return;
	}

	if (KEY::GetIns().GetInfo(KEY_TYPE::ENTER).down) {
		SceneManager::GetIns().ChangeScene(SCENE_ID::GAME);
		return;
	}
	skyDome_->Update();
}
void TitleScene::Draw(void)
{
	skyDome_->Draw();

	DrawString(0, 0, "タイトルシーン", 0xffffff);
}
void TitleScene::Release(void)
{
	if (skyDome_) {
		skyDome_->Release();
		delete skyDome_;
		skyDome_ = nullptr;
	}
}