#pragma once

class FPS;

class Application
{
public:
	//スクリーンサイズ
	static constexpr int SCREEN_SIZE_X = 1440;
	static constexpr int SCREEN_SIZE_Y = 810;

public:

	//シングルトン(生成・取得・削除)
	static void CreateIns(void) { if (ins == nullptr) { ins = new Application(); ins->Init(); } }
	static Application& GetIns(void) { return *ins; }
	static void DeleteIns(void) { if (ins != nullptr) delete ins; ins = nullptr; }

public :

	void Init(void);		//初期化
	void Run(void);			//ゲームループの開始
	void Release(void);		//解放

	//初期化成功/失敗の判定
	bool IsInitFail(void) const { return isInitFail; }

	//解放成功/失敗の判定
	bool IsReleaseFail(void) const { return isReleaseFail; }

	//ゲーム終了
	void GameEnd(void) { gameEnd = true; };

	//デバッグ描画（true = 描画する、false = 描画しない）
	bool IsDrawDebug(void) const { return drawDebug; }
	//デバッグ描画の設定（true = 描画するに設定、false = 描画しないに設定）
	void SetDrawDebug(bool flg) { drawDebug = flg; }
	//デバッグ描画の切り替え（現在の逆に切り替える）
	void DrawDebugSwitch(void) { drawDebug = !drawDebug; }


private:

	//デフォルトコンストラクタをprivateにして
	//外部から生成できないようにする
	Application(void);

	//デストラクタも同様
	~Application(void);

	//コピー・ムーブ操作を禁止
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

private:

	//静的インスタンス
	static Application* ins;

	//初期化失敗
	bool isInitFail;

	//解放失敗
	bool isReleaseFail;

	//ゲーム終了
	bool gameEnd;

	//FPS
	FPS* fps;

	//デバッグ描画（true = 描画する、false = 描画しない）
	bool drawDebug;

};

//Applicationの省略形
using App = Application;
