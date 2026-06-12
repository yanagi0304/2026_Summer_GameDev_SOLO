#pragma once
#include "../ActorBase.h"
#include "../Common/AnimationController/AnimationController.h"
#include "../../Application/Application.h"
#include <map>
#include <memory>
#include <optional>


class WeaponBase :
    public ActorBase
{
public:

	// デフォルトコンストラクタ
    WeaponBase();
	// パラメーターを外部から読み込む場合に使うコンストラクタ
    WeaponBase(const std::string& parameterPath);

	~WeaponBase() override = default;

	void Load(void) override;

private:

	//初期化
	void SubInit(void) override;
	//更新
	void SubUpdate(void) override;
	//描画
	void SubDraw(void) override;
	void SubAlphaDraw(void) override;
	//解放
	void SubRelease(void) override;

	//モデルのデフォルトカラー保存用配列
	std::vector<COLOR_F> DEFAULT_COLOR;

	//アニメーションコントローラーのインスタンス
	AnimationController* anime;

protected:

	//武器固有の処理
	virtual void WeaponLoad(void) = 0;
	virtual void WeaponInit(void) = 0;
	virtual void WeaponUpdate(void) = 0;
	virtual void WeaponDraw(void) = 0;
	virtual void WeaponAlphaDraw(void) = 0;
	virtual void WeaponUiDraw(void) = 0;
	virtual void WeaponRelease(void) = 0;

#pragma region アニメーションコントローラー

	//アニメーションコントローラーの作成
	void CreateAnimationController(void) { if (anime == nullptr) anime = new AnimationController(trans.model); }

	/// <summary>
	/// モデルにくっついているFBXアニメーションを全部登録する
	/// </summary>
	/// <param name="inFbxMaxIndex">くっついてるアニメーションの数</param>
	/// <param name="speed">再生速度</param>
	void AddInFbxAnimation(int inFbxMaxIndex, float speed);

	void AnimePlay(int type, bool loop = true) { anime->Play(type, loop); }

	bool IsAnimeEnd(void)const { return anime->GetAnimEnd(); }

	float GetAnimeRatio(void)const { return anime->GetAnimeRatio(); }

	int GetAnimePlayType(void)const { return anime->GetPlayType(); }

#pragma endregion

#pragma region セッター関連

	//武器を持たせる場所を指定
	void SetHavePos(const Vector3& pos) { trans.pos = pos; }


#pragma endregion

	std::optional<std::reference_wrapper<const Transform>> ownerTrans_;

};

