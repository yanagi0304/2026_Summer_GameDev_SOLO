#pragma once
#include<map>
#include<string>


class AnimationController
{
public:

	// アニメーションデータ
	struct Animation
	{
		int model = -1;
		int attachNo = -1;
		int animIndex = 0;
		float speed = 0.0f;
		float totalTime = 0.0f;
		float step = 0.0f;
	};

	// コンストラクタ
	AnimationController(int modelId);

	// デストラクタ
	~AnimationController(void);

	//外部FBXからアニメーションを追加
	void Add(int type, float speed, const std::string path);

	// 同じFBX内のアニメーションを準備
	void AddInFbx(int type, float speed, int animIndex);

	// アニメーション再生
	void Play(int type, bool loop = true);
	void Stop(void);
	void Update(void);
	void Release(void);

	bool IsEnd(void)const;
	int GetPlayType(void)const { return playType; }
	bool GetAnimEnd(void)const { return playAnim.step >= playAnim.totalTime; }

	float GetAnimeRatio(void)const { return (playAnim.step / playAnim.totalTime); }

	// 再生ステップを取得
	float GetStep(void)const { return playAnim.step; }
	// 再生ステップを設定
	void SetStep(float step);

private:
	// アニメーションするモデルのハンドルID
	int modelId;

	// 種類別のアニメーションデータ
	std::map<int, Animation> animations;

	// 再生中のアニメーション
	int playType;
	Animation playAnim;
	Animation prevAnim;
	bool loopflg;

	//アニメーションブレンドの割合
	float blendRate;

	//デタッチできているか
	int isDetach;

	void Add(int type, float speed, Animation animation);
};