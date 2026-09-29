#pragma once

#include "../Common/ActorBase/ActorBase.h"

#include <map>
#include <memory>
#include <optional>

class WeaponBase : public ActorBase
{
public:

	// デフォルトコンストラクタ
	WeaponBase();
	// パラメーターを外部から読み込む場合に使うコンストラクタ
	WeaponBase(const std::string& parameterPath);

	~WeaponBase() override = default;

	//所持武器専用アニメーション
	virtual int GetIdleAnimeID(void) const = 0;
	virtual int GetAttackAnimeID(void) const = 0;

private:

	//更新
	void BaseUpdate(void) override;
	//解放
	void BaseRelease(void) override;

	//アニメーションコントローラーのインスタンス
	AnimationController* anime;

protected:

#pragma region アニメーションコントローラー

	// アニメーションコントローラーの作成
	void CreateAnimationController(void);

	/// <summary>
	/// モデルにくっついてるFBXアニメーションを全部登録する
	/// </summary>
	/// <param name="inFbxMaxIndex">くっついてるアニメーションの数</param>
	/// <param name="speed">再生速度</param>
	/// <param name="loop">ループ再生フラグ配列（指定なしで全てループ再生有効で登録）</param>
	void AddInFbxAnimation(int inFbxMaxIndex, const float speed, const bool* const loop = nullptr);

	/// <summary>
	/// モデルにくっついてるFBXアニメーションを全部登録する
	/// </summary>
	/// <param name="inFbxMaxIndex">くっついてるアニメーションの数</param>
	/// <param name="speed">再生速度配列</param>
	/// <param name="loop">ループ再生フラグ配列（指定なしで全てループ再生有効で登録）</param>
	void AddInFbxAnimation(int inFbxMaxIndex, const float* const speed, const bool* const loop = nullptr);

	/// <summary>
	/// 別ファイルに保存されているFBXアニメーションを登録する
	/// </summary>
	/// <param name="index">参照番号</param>
	/// <param name="speed">再生速度</param>
	/// <param name="loop">ループ再生フラグ</param>
	/// <param name="filePath">パス</param>
	void AddAnimation(int index, float speed, bool loop, const char* filePath);

	/// <summary>
	/// アニメーション再生
	/// </summary>
	/// <param name="type">参照番号</param>
	/// <param name="loop">ループ再生フラグ（指定なしで登録された情報で再生）</param>
	virtual void AnimePlay(int type, signed char loop = -1);

	/// <summary>
	/// アニメーション再生
	/// </summary>
	/// <param name="type">参照番号</param>
	/// <param name="loop">ループ再生フラグ（指定なしで登録された情報で再生）</param>
	template<typename AnimeEnum>
	void AnimePlay(AnimeEnum type, signed char loop = -1) { AnimePlay(static_cast<int>(type), loop); }

	// アニメーション終了チェック（true = 最後まで再生し終わっている、false = まだ再生中）
	bool IsAnimeEnd(void)const;

	// アニメーションの再生比率を取得（0.0f～1.0f）
	float GetAnimeRatio(void)const;

	// アニメーションの再生時間を取得
	float GetAnimeTotalTime(void)const;

	// アニメーションの再生タイプを取得（0 = ループ、1 = 1回再生）
	int GetAnimePlayType(void)const;

	// アニメーションの再生時間を取得（0.0f～再生時間）
	float GetAnimeStep(void)const;

	// アニメーションの再生時間を設定（0.0f～再生時間）
	void SetAnimeStep(float step);

#pragma endregion

#pragma region セッター関連

	//武器を持たせる場所を指定
	void SetHavePos(const Vector3& pos) { trans.pos = pos; }

#pragma endregion

	std::optional<std::reference_wrapper<const Transform>> ownerTrans_;
};

