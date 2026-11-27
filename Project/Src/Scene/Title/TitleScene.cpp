#include "TitleScene.h"

#include "../../Application/Application.h"

#include "../../Manager/Input/InputManager.h"
#include "../../Manager/Sound/SoundManager.h"
#include "../../Manager/Font/FontManager.h"

#include "../SceneManager.h"

#include "../../Object/Common/AnimationController/CharacterAnimeTypeDefine.h"

namespace
{
	// ランダム再生するアニメーション
	constexpr int RANDOM_ANIMES[] = {
		CHARACTER_ANIME::IDLE,
		CHARACTER_ANIME::WALK,
		CHARACTER_ANIME::RUN,
		CHARACTER_ANIME::JUMPING,
		CHARACTER_ANIME::NinjaRun,
	};

	void PlayRandamAnim(AnimationController& anime)
	{
		const int count = static_cast<int>(std::size(RANDOM_ANIMES));
		int next = 0;
		next = RANDOM_ANIMES[GetRand(count - 1)];
		anime.Play(next);
	}
}

TitleScene::TitleScene() : SceneBase(),
	background{},
	titleLogo{},
	anime(nullptr)
{
}

void TitleScene::SubPostLoad(void)
{
	// 音声のシーン切り替え
	Snd::GetIns().ChangeScene("Title");

#pragma region 画像の設定

	// 背景画像の読み込み
	background.Load("Data/Image/Title/TitleBackImage.png");
	background.size = { App::SCREEN_SIZE_X,App::SCREEN_SIZE_Y };

	// タイトルロゴの設定
	titleLogo.Load("Data/Image/Title/TitleLogo.png");
	titleLogo.pos = { 426,196 };
	titleLogo.scale = 0.45;

#pragma endregion

	// モデルの読み込み
	trans.LoadModel("Character/WoChamolenium/WoChamolenium");

}

void TitleScene::SubPostInit(void)
{
	// シェーダーの初期化
	hologramShader.Init();

	// モデルの大きさと位置
	trans.scale = Vector3(1.75f, 1.75f, 1.75f);
	trans.pos = Vector3(130.0f, 0.0f, 0.0f);

	// 待機アニメーションをループ再生
	static constexpr float IDLE_ANIME_SPEED = 0.5f;
	anime = new AnimationController(trans.model);
	anime->AddInFbx(CHARACTER_ANIME::IDLE, IDLE_ANIME_SPEED, true, CHARACTER_ANIME::IDLE);
	anime->AddInFbx(CHARACTER_ANIME::WALK, IDLE_ANIME_SPEED, true, CHARACTER_ANIME::WALK);
	anime->AddInFbx(CHARACTER_ANIME::RUN, IDLE_ANIME_SPEED, true, CHARACTER_ANIME::RUN);
	anime->AddInFbx(CHARACTER_ANIME::NinjaRun, IDLE_ANIME_SPEED, true, CHARACTER_ANIME::NinjaRun);
	anime->AddInFbx(CHARACTER_ANIME::JUMPING, IDLE_ANIME_SPEED, true, CHARACTER_ANIME::JUMPING);
	PlayRandamAnim(*anime);
}

void TitleScene::SubPostUpdate(void)
{
	// ゲーム終了処理
	if (Input::GetIns().GetInfo(KEY_TYPE::End).down) {
		App::GetIns().GameEnd();
	}

	// 決定
	if (Input::GetIns().GetInfo(KEY_TYPE::Enter).down) {
		SceneManager::GetIns().ChangeSceneFade(SCENE_ID::Game);
	}

	static constexpr float MOVE_POWER = 2.0f;
	//if (Input::GetIns().GetInfo(Input::KEY_TYPE::PlayerMoveFront).now) { titleLogo.pos.y -= MOVE_POWER; }
	//if (Input::GetIns().GetInfo(Input::KEY_TYPE::PlayerMoveBack).now) { titleLogo.pos.y += MOVE_POWER; }
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveLeft).now) { trans.AddAngleYDeg(MOVE_POWER); }
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMoveRight).now) { trans.AddAngleYDeg(-MOVE_POWER); }


	// アニメーションの更新
	anime->Update();

	// シェーダーの更新
	hologramShader.Update();

}

void TitleScene::SubUiDraw(void)
{

	// 背景画像の描画
	background.Draw();
	// タイトルロゴの描画
	titleLogo.DrawRota();

	SetCameraNearFar(1.0f, 5000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 90.0f, -260.0f), VGet(0.0f, 90.0f, 0.0f));

	// モデルをホログラムで描画
	hologramShader.DrawModel(trans);

	DrawStringToHandle(0, 0, "タイトル", 0xffffff, Font::GetIns().GetFont(FontKinds::Marumiya40));


}

void TitleScene::SubPostRelease(void)
{
	// 画像の解放
	background.Release();
	titleLogo.Release();

	// アニメーションの解放(モデルより先に行う)
	if (anime != nullptr) {
		anime->Release();
		delete anime;
		anime = nullptr;
	}

	// モデルの解放
	trans.Release();

	// シェーダーの解放
	hologramShader.Release();
}
