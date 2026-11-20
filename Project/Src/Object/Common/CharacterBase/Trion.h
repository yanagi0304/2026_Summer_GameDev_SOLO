#include "CharacterBase.h"
#pragma once

class Trion
{
public:

	Trion() = default;
	~Trion() = default;

	void Load(void);
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

#pragma region ゲッター・セッター

	float GetTrionMax(void)const { return trionMax; }
	float GetTrionCurrent(void)const { return trionCurrent; }
	void SetMax(float trionMax) { this->trionMax = trionMax; }
	
	void SetTrionCurrent(float trionCurrent) { this->trionCurrent = trionCurrent; }

	// ダメージ・トリオン消費
	bool Decrease(const float decrease);

	// 残りのトリオンの割合
	float GetRatio(void)const { return trionCurrent / trionMax; }

	// 死亡判定
	const bool IsDead(void)const { return trionCurrent <= 0.0f; }

	// 死んだ際の処理
	void OnDead(void);



#pragma endregion


private:

	float trionMax;
	float trionCurrent;

};

