#include "Application.h"

#include <DxLib.h>

#include "../Manager/FPS/FPS.h"
#include "KeyManager.h"

Application* Application::ins = nullptr;

Application::Application(void) :
	isInitFail(false),
	isReleaseFail(false),
	gameEnd(false),
	drawDebug(false),
	fps(nullptr)
{

}

Application::~Application(void)
{
}

void Application::Init(void)
{
	//アプリケーションの初期化設定
	SetWindowText("");

	//ウィンドウ関連
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);	//サイズ変更
	ChangeWindowMode(false);
#ifdef _DEBUG
	ChangeWindowMode(true);
#endif // DEBUG

	//Dxlibの初期化
	isInitFail = false;
	if (DxLib_Init() == -1) { isInitFail = true; return; }

	//描画先画面を裏にする
	SetDrawScreen(DX_SCREEN_BACK);

	//キー制御初期化
	SetUseDirectInputFlag(true);

	//FPS初期化
	fps = new FPS();
	fps->Init();

}

void Application::Run(void)
{
	// ゲームループ
	while (ProcessMessage() == 0 && !gameEnd)
	{
		//フレームレート上限まで経過していないなら
		// 再ループさせる
		if (!fps->UpdateFrameRate()) { continue; }

		//フレームレートの計算
		fps->CalcFrameRate();

		// 背面描画画面をクリア
		ClearDrawScreen();

#ifdef _DEBUG
		//フレームレートデバッグ描画
		fps->DrawFrameRate();
#endif // _DEBUG


		// 描画が完了した背面画面を表に持ってくる
		ScreenFlip();
	}
}

bool KeyManager::GetControllerConnect(void) const
{
	return false;
}

Vector2 KeyManager::GetRightStickVec(void) const
{
	return Vector2();
}

Vector2 KeyManager::GetLeftStickVec(void) const
{
	return Vector2();
}

void Application::Release(void)
{

	//フレームレート解放
	delete fps;

	if (DxLib_End() == -1) { isReleaseFail = true; }
}


