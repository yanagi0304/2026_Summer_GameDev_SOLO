#pragma once
#include "../ActorBase.h"
#include "../Common/AnimationController/AnimationController.h"
#include "CharactorStateBase.h"

#include <map>

class CharactorBase : public ActorBase
{
public:
	// デフォルトコンストラクタ
	CharactorBase();
	// パラメーターを外部から読み込む場合に使うコンストラクタ
	CharactorBase(const std::string& parameterPath);
	virtual ~CharactorBase()override = default;

	// ステートのゲット関数
	int GetState(void)const { return state; }

private:
	// 初期化
	void SubInit(void)override;
	// 更新
	void SubUpdate(void)override;
	// 描画
	void SubDraw(void)override;
	void SubAlphaDraw(void)override;
	// 解放
	void SubRelease(void)override;

	// モデルデフォルトカラー保存用配列
	std::vector<COLOR_F> DEFAULT_COLOR;

	// アニメーションコントローラーのインスタンス
	AnimationController* anime;

	// 無敵カウンター
	unsigned char inviCounter;
	
	// 無敵カウンターの更新
	void Invi(void);

	// 無敵カウンターによるダメージ演出を行うかどうか（true = 「する」、false = 「しない」）← デフォルトは「する」
	bool isInviEffect;

	// ステート管理用マップ（キー：ステート番号、値：状態クラスのポインタ）
	std::map<int, CharactorStateBase*> stateMap;

protected:
	// ステート管理用変数
	int state;

	// ステートの追加
	void AddState(int stateNum, CharactorStateBase* stateIns) { stateMap.emplace(stateNum, stateIns); }

	// 指定のステートインスタンスをゲットする関数
	CharactorStateBase& GetStateIns(int state) {
		auto it = stateMap.find(state);
		if (it != stateMap.end()) { return *(it->second); }
		else { throw std::runtime_error("指定のステートインスタンスが見つかりません"); }
	}

	// キャラクター固有の処理をここに追加
	virtual void CharactorInit(void) = 0;
	virtual void CharactorUpdate(void) = 0;
	virtual void CharactorDraw(void) = 0;
	virtual void CharactorAlphaDraw(void) = 0;
	virtual void CharactorRelease(void) = 0;

#pragma region アニメーションコントローラー
	// アニメーションコントローラーの作成
	void CreateAnimationController(void) { if (anime == nullptr) anime = new AnimationController(trans.model); }
	
	/// <summary>
	/// モデルにくっついてるFBXアニメーションを全部登録する
	/// </summary>
	/// <param name="inFbxMaxIndex">くっついてるアニメーションの数</param>
	/// <param name="speed">再生速度</param>
	void AddInFbxAnimation(int inFbxMaxIndex, float speed);

	void AddInFbxAnimation(int inFbxMaxIndex, const float* speed);

	/// <summary>
	/// 別ファイルに保存されているFBXアニメーションを登録する
	/// </summary>
	/// <param name="index">参照番号</param>
	/// <param name="speed">再生速度</param>
	/// <param name="filePath">パス</param>
	void AddAnimation(int index, float speed, const char* filePath);

	void AnimePlay(int type, bool loop = true) { anime->Play(type, loop); }

	bool IsAnimeEnd(void)const { return anime->GetAnimEnd(); }

	float GetAnimeRatio(void)const { return anime->GetAnimeRatio();}

	int GetAnimePlayType(void)const { return anime->GetPlayType(); }
	
#pragma endregion オーバーライド不可(使用のみ)

	// 無敵カウンターのゲット関数
	unsigned char GetInviCounter(void)const { return inviCounter; }
	// 無敵カウンターのセット関数
	void SetInviCounter(unsigned char counter = 1) { inviCounter = (counter < 0) ? 0 : (counter > 255) ? 255 : counter; }

	// 無敵演出フラグのセット関数（true = 「する」、false = 「しない」）← デフォルトは「する」
	void SetInviEffectFlg(bool flg = true) {
		if (isInviEffect == flg) { return; }

		isInviEffect = flg;

		// 無敵演出フラグが変わったとき色をデフォルトに戻す

		// 無敵演出フラグが「する」に変わったときはモデルデフォルトカラーを保存しておく
		int mnum = MV1GetMaterialNum(trans.model);
		for (int i = 0; i < mnum; ++i) {
			COLOR_F emi = MV1GetMaterialEmiColor(trans.model, i);

			emi.r = (std::min)(emi.r + 0.4f, 1.0f);
			emi.g = (std::min)(emi.g + 0.4f, 1.0f);
			emi.b = (std::min)(emi.b + 0.4f, 1.0f);

			// モデルデフォルトカラーの保存
			if (isInviEffect) { DEFAULT_COLOR.emplace_back(emi); }

			MV1SetMaterialEmiColor(trans.model, i, emi);
		}

		// 「しない」に変わったときは必要ないので情報を破棄する
		if (!isInviEffect) { DEFAULT_COLOR.clear(); }
	}

};