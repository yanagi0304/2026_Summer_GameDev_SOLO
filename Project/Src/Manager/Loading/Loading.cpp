#include "Loading.h"

#include "../../pch.h"

#include"../../Application/Application.h"
#include"../../Utility/Utility.h"

Loading* Loading::ins = nullptr;

// コンストラクタ
Loading::Loading()
	: isLoading(false)
	, loadTimer(0)
	, animInterval()
	, animCounter()
	, runImages()
	, runFrame(0)
	, runInterval(0)
{}

// デストラクタ
Loading::~Loading()
{}

// 初期化
void Loading::Init(void)
{
	loadTimer = 0;
	isLoading = false;

	animCounter = 0;
	animInterval = 0;
	runFrame = 0;
	runInterval = 0;
}

// 読み込み
void Loading::Load(void)
{
	// 非同期読み込みに切り替える前に、ロード画面用の画像を読み込んでおく
	LoadDivGraph("Data/Image/Loading/LoadingRun.png", RUN_FRAME_NUM, RUN_FRAME_NUM, 1,
		RUN_FRAME_SIZE, RUN_FRAME_SIZE, runImages);
}

// 更新
void Loading::Update(void)
{
	loadTimer++;

	// 読込中のものがなくなったら or 最低ロード時間経過
	if (GetASyncLoadNum() == 0 && loadTimer >= MIN_LOAD_TIME)
	{
		// ロード終了
		Init();
	}
	// 読み込み中
	else
	{
		// 走りアニメーションのコマ送り
		if (++runInterval >= RUN_FRAME_INTERVAL) {
			runInterval = 0;
			runFrame = (runFrame + 1) % RUN_FRAME_NUM;
		}

		// ロード画面を動作させるならここに記述
		if (++animInterval >= 10) {
			animInterval = 0;
			if (++animCounter >= 4) {
				animCounter = 0;
			}
		}
	}
}

// 描画
void Loading::Draw(void)
{
	using app = Application;
	int xx = app::SCREEN_SIZE_X;
	int yy = app::SCREEN_SIZE_Y;
	int x = xx / 2;
	int y = yy / 2;

	DrawBox(0, 0, xx, yy, 0x000000, true);
	std::string st = "Loading";
	for (int i = 0; i < animCounter; i++) { st += "."; }
	int fontSize = 45;
	SetFontSize(fontSize);
	DrawString(xx - fontSize * 6, yy - fontSize, st.c_str(), 0xffffff);
	SetFontSize(16);

	// 文字の左隣で、左右反転して縮小表示する
	double scale = static_cast<double>(RUN_DRAW_SIZE) / RUN_FRAME_SIZE;
	SetDrawMode(DX_DRAWMODE_BILINEAR);
	DrawRotaGraph(xx - fontSize * 6 + RUN_OFFSET_X, yy + RUN_OFFSET_Y,
		scale, 0.0, runImages[runFrame], true, true);
	SetDrawMode(DX_DRAWMODE_NEAREST);
}

// 解放
void Loading::Release(void)
{
	for (int& image : runImages) {
		DeleteGraph(image);
		image = -1;
	}
}

// 非同期読み込みに切り替える
void Loading::StartAsyncLoad(void)
{
	isLoading = true;
	// 非同期読み込み開始
	SetUseASyncLoadFlag(true);
}

// 同期読み込みに切り替える
void Loading::EndAsyncLoad(void)
{
	// 非同期読み込み終了
	SetUseASyncLoadFlag(false);
}
