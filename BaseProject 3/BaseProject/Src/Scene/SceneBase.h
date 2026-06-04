#pragma once

#include "../Object/ActorBase.h"

class SceneBase
{
public:
	// コンストラクタ
	SceneBase(void) {};

	// デストラクタ
	virtual ~SceneBase(void) = 0 {};

	// 読み込み
	virtual void Load(void) = 0;

	// 初期化処理
	virtual void Init(void) = 0;

	// 更新ステップ
	virtual void Update(void) = 0;

	// 描画処理
	virtual void Draw(void) = 0;

	// 解放処理
	virtual void Release(void) = 0;

protected:

	// オブジェクト格納用の配列
	std::vector<ActorBase*>objects;

	// 配列の中から特定のオブジェクトを探す
	template<typename T>
	T* ObjSerch(void) {
		for (auto* obj : objects) {
			if (dynamic_cast<T*>(obj)) { return dynamic_cast<T*>(obj); }
		}
		return nullptr;
	}
	template<typename T = ActorBase>
	std::vector<T*> ObjArraySerch(void) {
		std::vector<T*> objArray;
		for (auto* obj : objects) {
			if (dynamic_cast<T*>(obj)) { objArray.emplace_back(dynamic_cast<T*>(obj)); }
		}
		return objArray;
	}
};
