#include "WeaponBase.h"

WeaponBase::WeaponBase()
	:
	anime(nullptr),
	DEFAULT_COLOR()
{
}

WeaponBase::WeaponBase(const std::string& parameterPath)
	:
	ActorBase(parameterPath),
	anime(nullptr),
	DEFAULT_COLOR()
{
}

void WeaponBase::Load(void)
{
	WeaponLoad();
}

void WeaponBase::SubInit(void)
{
	WeaponInit();
}

void WeaponBase::SubUpdate(void)
{
	WeaponUpdate();

	// アニメーション更新
	if (anime) { anime->Update(); }
}

void WeaponBase::SubDraw(void)
{
	WeaponDraw();
}

void WeaponBase::SubAlphaDraw(void)
{
	WeaponAlphaDraw();
}

void WeaponBase::SubRelease(void)
{
	WeaponRelease();

	// デフォルトカラー情報の解放
	if (!DEFAULT_COLOR.empty()) {
		DEFAULT_COLOR.clear();
	}

	// アニメーションコントローラーの解放（使われていたら）
	if (anime) {
		anime->Release();
		delete anime;
		anime = nullptr;
	}

}
