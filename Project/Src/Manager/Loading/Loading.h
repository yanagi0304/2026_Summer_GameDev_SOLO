#pragma once

class Loading
{
public:
	// シングルトン（生成・取得・削除）
	static void CreateInstance(void) { if (ins == nullptr) { ins = new Loading(); } };
	static Loading* GetInstance(void) { return ins; };
	static void DeleteInstance(void) { if (ins != nullptr) { delete ins; ins = nullptr; } }

private:
	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	Loading();
	~Loading();

	// コピー・ムーブ操作を禁止
	Loading(const Loading&) = delete;
	Loading& operator=(const Loading&) = delete;
	Loading(Loading&&) = delete;
	Loading& operator=(Loading&&) = delete;

	// 下記をコンパイルエラーさせるため 上記を追加
	// Loading copy = *Loading::GetInstance();
	// Loading copied(*Loading::GetInstance());
	// Loading moved = std::move(*Loading::GetInstance());
public:

	void Init(void);		// 初期化
	void Load(void);		// 読み込み
	void Update(void);		// 更新
	void Draw(void);		// 描画
	void Release(void);		// 解放

	void StartAsyncLoad(void);	// 非同期ロードの開始
	void EndAsyncLoad(void);	// 非同期ロードの終了

	// ロード中かを返す。
	bool IsLoading(void) { return isLoading; }

private:

	// 静的インスタンス
	static Loading* ins;

	// 最低でもロード画面を表示する時間
	static constexpr int MIN_LOAD_TIME = 0;	// 60fps(1秒) * x

	// 走りアニメーションのコマ数
	static constexpr int RUN_FRAME_NUM = 8;

	// 走りアニメーション1コマの大きさ
	static constexpr int RUN_FRAME_SIZE = 196;

	// 走りアニメーションのコマ送り間隔(フレーム)
	static constexpr int RUN_FRAME_INTERVAL = 5;

	// 走りアニメーションを表示する大きさ
	static constexpr int RUN_DRAW_SIZE = 100;

	// 文字の左端を基準にしたキャラ中心の位置
	static constexpr int RUN_OFFSET_X = -40;

	// 画面下端を基準にしたキャラ中心の位置
	static constexpr int RUN_OFFSET_Y = -44;

	// 画像ハンドル
	int runImages[RUN_FRAME_NUM];

	// 走りアニメーションの現在コマと経過フレーム
	int runFrame;
	int runInterval;


	int animCounter;
	int animInterval;

	// ロード中の判定用
	bool isLoading;

	// 最低でもロード画面を表示する時間の範囲
	int loadTimer;
};
