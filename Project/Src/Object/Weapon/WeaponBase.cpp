#include "WeaponBase.h"

#include "../Common/AnimationController/AnimationController.h"

WeaponBase::WeaponBase()
	:
	anime(nullptr)
{
}

WeaponBase::WeaponBase(const std::string& parameterPath)
	:
	ActorBase(parameterPath),
	anime(nullptr)
{
}

void WeaponBase::BaseUpdate(void)
{
	// アニメーション更新
	if (anime) { anime->Update(); }
}

void WeaponBase::BaseRelease(void)
{
	// アニメーションコントローラーの解放（使われていたら）
	if (anime) {
		anime->Release();
		delete anime;
		anime = nullptr;
	}
}

#pragma region アニメーションコントローラー

void WeaponBase::CreateAnimationController(void) { if (anime == nullptr) anime = new AnimationController(trans.model); }

void WeaponBase::AddInFbxAnimation(int inFbxMaxIndex, float speed, const bool* const loop)
{
	for (int index = 0; index < inFbxMaxIndex; index++) {
		anime->AddInFbx(index, speed, (loop != nullptr) ? loop[index] : true, index);
	}
}

void WeaponBase::AddInFbxAnimation(int inFbxMaxIndex, const float* const speed, const bool* const loop)
{
	for (int index = 0; index < inFbxMaxIndex; index++) {
		anime->AddInFbx(index, speed[index], (loop != nullptr) ? loop[index] : true, index);
	}
}

void WeaponBase::AddAnimation(int index, float speed, bool loop, const char* filePath)
{
	anime->Add(index, speed, loop, filePath);
}

void WeaponBase::AnimePlay(int type, signed char loop)
{
	anime->Play(type, loop);
}

bool WeaponBase::IsAnimeEnd(void) const { return anime->IsAnimEnd(); }

float WeaponBase::GetAnimeRatio(void) const { return anime->GetAnimeRatio(); }

float WeaponBase::GetAnimeTotalTime(void) const { return anime->GetAnimeTotalTime(); }

int WeaponBase::GetAnimePlayType(void)const { return anime->GetAnimePlayType(); }

float WeaponBase::GetAnimeStep(void)const { return anime->GetAnimeStep(); }

void WeaponBase::SetAnimeStep(float step) { anime->SetAnimeStep(step); }

#pragma endregion