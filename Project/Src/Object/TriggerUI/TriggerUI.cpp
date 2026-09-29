#include "TriggerUI.h"

#include "../../pch.h"

#include "../../Application/Application.h"
#include "../../Manager/Input/InputManager.h"

TriggerUI::TriggerUI(const int& mainCurrentIndex_, const int& subCurrentIndex_) :

	mainCurrentIndex_(mainCurrentIndex_),
	subCurrentIndex_(subCurrentIndex_),

	triggerUI_(-1),
	select_(-1)
{
}

void TriggerUI::Load()
{
	//画像の読み込み
	triggerUI_ = LoadGraph("Data/Image/TriggerUI/TriggerUI.png");
	select_ = LoadGraph("Data/Image/TriggerUI/SelectUI.png");
	//武器アイコンの読み込み
	icon_.emplace(TriggerType::KOGETSU, LoadGraph("Data/Image/TriggerUI/Kogetsu.png"));
	icon_.emplace(TriggerType::GRASS_HOPPER, LoadGraph("Data/Image/TriggerUI/Grasshopper.png"));
	icon_.emplace(TriggerType::ASTEROID, LoadGraph("Data/Image/TriggerUI/Asteroid.png"));

	// このアクターは「UI」として描画する
	SetDrawType(ACTOR_DRAW_TYPE::Ui);
}

void TriggerUI::SubUpdate()
{
	static constexpr float MOVE_POWER = 2.0f;
	if (Input::GetIns().GetInfo(Input::KEY_TYPE::PlayerMoveFront).now) { y -= MOVE_POWER; }
	if (Input::GetIns().GetInfo(Input::KEY_TYPE::PlayerMoveBack).now) { y += MOVE_POWER; }
	if (Input::GetIns().GetInfo(Input::KEY_TYPE::PlayerMoveLeft).now) { x -= MOVE_POWER; }
	if (Input::GetIns().GetInfo(Input::KEY_TYPE::PlayerMoveRight).now) { x += MOVE_POWER; }
}

void TriggerUI::SubDraw()
{
	//メインUI
	DrawRotaGraph(SIZE_X / 2 + OFFSET,
		Application::SCREEN_SIZE_Y - SIZE_Y / 2 - OFFSET, 1.0f, 0.0f, triggerUI_, TRUE);

	//セレクトカーソル
	//サブトリガー
	DrawRotaGraph(
		116,
		600 + (subCurrentIndex_ * 50),
		1.0,
		0.0,
		select_,
		TRUE);
	//メイントリガー
	DrawRotaGraph(
		363,
		600 + (mainCurrentIndex_ * 50),
		1.0,
		0.0,
		select_,
		TRUE);

	for (int i = 0; i < mainTrig_.size(); i++)
	{
		DrawFormatString(363-84, 600 + (i*50), 0xffffff, TriggerToString(mainTrig_[i]));
		//アイコンを表示
		auto it = icon_.find(mainTrig_[i]);
		if (it != icon_.end() && it->second != -1)
		{
			DrawRotaGraph(479, 606 + (i * 50), 1.0f, 0.0f, it->second, false);
		}

	}

	DrawFormatString(0, 100, 0xffffff, "%d,%d", x, y);

}

const char* TriggerUI::TriggerToString(TriggerType trigger)
{
	switch (trigger)
	{
	case TriggerType::ASTEROID:
		return "Asteroid";

	case TriggerType::KOGETSU:
		return "Kogetsu";

	case TriggerType::GRASS_HOPPER:
		return "GrassHopper";

	case TriggerType::HOUND:
		return "Hound";

	case TriggerType::VIPER:
		return "Viper";

	case TriggerType::SHIELD:
		return "Shield";

	default:
		return "None";
	}
}