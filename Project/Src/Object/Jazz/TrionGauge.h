#pragma once

#include <algorithm>

// トリオン(戦うためのエネルギー)
// このゲームでは、プレイヤーの体力として扱う。
//   - 敵の攻撃を受けると漏れて(Leak)減る。受けた直後は、短い無敵時間があり、続けて減らない
//   - アステロイドなど武器を使う時に、消費(Consume)する使い方も想定している(今は未使用)
//   - 0になると戦えなくなる = ゲームオーバー
class TrionGauge
{
public:

	/// <param name="maxValue">最大値</param>
	/// <param name="invincibleFrames">攻撃を受けた後の無敵時間(フレーム)</param>
	explicit TrionGauge(float maxValue = 100.0f, float invincibleFrames = 60.0f) :
		maxValue(maxValue),
		value(maxValue),
		invincibleFrames(invincibleFrames),
		invincibleLeft(0.0f),
		flashLeft(0.0f)
	{
	}

	// 毎フレーム呼ぶ(framesは、時間倍率を掛けた経過フレーム数)
	void Update(float frames)
	{
		invincibleLeft = (std::max)(0.0f, invincibleLeft - frames);
		flashLeft = (std::max)(0.0f, flashLeft - frames);
	}

	// 攻撃を受けて、トリオンが漏れる。減らしたらtrue。無敵時間中や、すでに0ならfalse
	bool Leak(float amount)
	{
		if (invincibleLeft > 0.0f || value <= 0.0f) { return false; }
		value = (std::max)(0.0f, value - amount);
		invincibleLeft = invincibleFrames;
		flashLeft = FLASH_FRAMES;
		return true;
	}

	// 武器の使用などで消費する(無敵時間は発生しない)。消費できたらtrue
	bool Consume(float amount)
	{
		if (value < amount) { return false; }
		value -= amount;
		return true;
	}

	// 回復する
	void Recover(float amount) { value = (std::min)(maxValue, value + amount); }

	float GetValue(void)const { return value; }
	float GetMax(void)const { return maxValue; }
	float GetRatio(void)const { return maxValue > 0.0f ? value / maxValue : 0.0f; }
	bool IsEmpty(void)const { return value <= 0.0f; }
	bool IsInvincible(void)const { return invincibleLeft > 0.0f; }

	// 被弾の演出用(1.0から0.0へ減っていく。攻撃を受けた瞬間が1.0)
	float GetFlashRatio(void)const { return flashLeft / FLASH_FRAMES; }

private:

	// 被弾の演出を出すフレーム数
	static constexpr float FLASH_FRAMES = 20.0f;

	float maxValue;
	float value;
	float invincibleFrames;
	float invincibleLeft;
	float flashLeft;
};
