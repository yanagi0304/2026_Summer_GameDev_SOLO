#pragma once

#include <vector>

#include "../../Utility/Utility.h"

#include "../Common/Collider/ColliderTagDefine.h"

// ダメージ計算式（計算式忘れた、後で書く）
static short CalculateDamage(short damage, short defense) { return Round((float)damage / (((float)defense + 100.0f) / 100.0f)); }


// バフ/デバフ の最大数
static constexpr char MODIFIER_MAX_NUM = 10;

// デバフの数値上限
static constexpr float DEBUFF_MAX = 0.01f;


// 補正倍率(バフ/デバフ)の構造体
struct ModifierData
{
	// タイプ（同タイプの重複不可）
	COLLIDER_TAG type;
	// 増減率(バフ/デバフ)の数値（0.0が基準値）（生成関数を通して基準値を補正 例:80->0.8 -80->-0.8）
	float rate;
	// 効果時間（フレーム数）
	short time;

	// 効果時間の更新（戻り値：「true」= 効果時間継続、「false」= 効果時間終了）
	bool TimeUpdate(void) {
		if (--time > 0) { return true; }
		else { return false; }
	}

	/// <summary>
	/// 生成
	/// </summary>
	/// <param name="type">タイプ（同タイプの重複不可）</param>
	/// <param name="rate">補正倍率(バフ/デバフ)の数値（0が基準値 例:80->1.8倍 -80->0.2倍）</param>
	/// <param name="time">効果時間（フレーム数）</param>
	ModifierData(COLLIDER_TAG type, short rate, short time) : type(type), rate(PercentConversion(rate)), time(time) {}
};


struct CharacterStatsValue
{
private:
	// ベース
	const short BASE;

	// 補正倍率(バフ/デバフ)を1つずつ格納する配列
	std::vector<ModifierData> modifier;

	// 補正倍率(バフ/デバフ)の総量
	float TotalMag(void)const {
		// 基準値
		float total = 1.0f;

		// 加減算
		for (const ModifierData& m : modifier) { total += m.rate; }

		// デバフ上限を加味して数値を返却
		return (total > DEBUFF_MAX) ? total : DEBUFF_MAX;
	}
public:
	// 実数値
	short Value(void)const { return (short)Round((float)BASE * TotalMag()); }

	// バフ/デバフ をかける
	void AddModifier(ModifierData add) {
		// すでに同じ種類がかかっている場合は上書き
		for (ModifierData& mod : modifier) {
			if (add.type == mod.type) { mod = add; return; }
		}

		// 最大数を超える場合は追加なし
		if (modifier.size() >= MODIFIER_MAX_NUM) { return; }

		// 末尾に バフ/デバフ を追加
		modifier.emplace_back(add);
	}

	// バフ/デバフ の効果時間の更新
	void ModifinerTimeUpdate(void) {
		modifier.erase(
			std::remove_if(
				modifier.begin(),
				modifier.end(),
				[](ModifierData& mod) { return !mod.TimeUpdate(); }
			),
			modifier.end()
		);
	}

	/// <summary>
	/// 生成
	/// </summary>
	/// <param name="BASE">ベース数値</param>
	CharacterStatsValue(short BASE) : BASE(BASE), modifier() { modifier.reserve(MODIFIER_MAX_NUM); }
};


// 会心・非会心 を算出するためのランダム数値の最大値
static constexpr int CRITICAL_RAND_MAX = 10000;

// デフォルト会心率(単位:%)
static constexpr short DEFAULT_CRITICAL_RATE = 10;
// デフォルト会心ダメージ(単位:%)
static constexpr short DEFAULT_CRITICAL_DAMAGE = 150;

// 会心率ダメ構造体
struct Critical
{
	// 会心率
	CharacterStatsValue rate;
	// 会心ダメージ
	CharacterStatsValue damage;

	// 会心判定
	bool IsCritical(void)const {
		return (GetRand(CRITICAL_RAND_MAX) <= CRITICAL_RAND_MAX * PercentConversion(rate.Value()));
	}
	// 判定を含めた最終的な倍率
	float ResultDamageRate(bool* const isCritical)const {
		// アウトプット引数の指定がなければ会心判定を保存せず数値のみを返す
		if (isCritical == nullptr) {
			return (IsCritical()) ? PercentConversion(damage.Value()) : 1.0f;
		}
		// アウトプット引数の指定があれば会心判定をアウトプット引数に保存して数値を返す
		else {
			*isCritical = IsCritical();
			return (*isCritical) ? PercentConversion(damage.Value()) : 1.0f;
		}
	}

	// バフ/デバフ の効果時間の更新
	void ModifinerTimeUpdate(void) {
		rate.ModifinerTimeUpdate();
		damage.ModifinerTimeUpdate();
	}

	// 生成
	Critical(bool IS_CRITICAL) : rate((IS_CRITICAL) ? DEFAULT_CRITICAL_RATE : 0), damage(DEFAULT_CRITICAL_DAMAGE) {}
};


struct CharacterStats
{
	// 最大ヒットポイント
	CharacterStatsValue hpMax;

	// ヒットポイント
	short hp;

	// 攻撃力
	CharacterStatsValue attackPower;

	// 会心率ダメ
	Critical critical;

	// 防御力
	CharacterStatsValue defensePower;

	// 速力
	CharacterStatsValue speedPower;

	// 全ての バフ/デバフ の効果時間を更新
	void ModifinerTimeUpdate(void) {
		hpMax.ModifinerTimeUpdate();
		attackPower.ModifinerTimeUpdate();
		critical.ModifinerTimeUpdate();
		defensePower.ModifinerTimeUpdate();
		speedPower.ModifinerTimeUpdate();
	}

	/// <summary>
	/// 生成
	/// </summary>
	/// <param name="HP_MAX">最大ヒットポイント</param>
	/// <param name="ATTACK_POWER">攻撃力</param>
	/// <param name="DEFENSE_POWER">防御力</param>
	/// <param name="SPEED_POWER">速力</param>
	CharacterStats(
		short HP_MAX,
		short ATTACK_POWER,
		short DEFENSE_POWER,
		short SPEED_POWER,
		bool IS_CRITICAL = true
	) :
		hpMax(HP_MAX),
		hp(hpMax.Value()),
		attackPower(ATTACK_POWER),
		critical(IS_CRITICAL),
		defensePower(DEFENSE_POWER),
		speedPower(SPEED_POWER)
	{
	}
};


struct SkillStats
{
private:
	// 技威力
	const short SKILL_POWER;

	// 攻撃力のポインタ
	const CharacterStatsValue* attackPower;

	// 会心率ダメのポインタ
	const Critical* critical;

	// この技が バフ/デバフ だった場合の効果時間
	const short SKILL_TIME;

public:
	// 威力（攻撃力を参照しない バフ/デバフ や 回復 などのスキルはそのまま技威力が実数値として返される）
	short Power(bool* const isCritical = nullptr)const {
		short ret = 0;
		if (attackPower == nullptr || critical == nullptr) { ret = SKILL_POWER; }
		else {
			ret = (short)Round(attackPower->Value() * PercentConversion(SKILL_POWER));
			ret = (short)Round((float)ret * critical->ResultDamageRate(isCritical));
		}
		return ret;
	}

	// コライダータグ
	const COLLIDER_TAG COLL_TAG;

	/// <summary>
	/// 攻撃/回復 スキル生成
	/// </summary>
	/// <param name="SKILL_POWER">技威力</param>
	/// <param name="characterStats">ステータスのポインタ（回復などの攻撃力や会心率ダメを参照しないスキルの場合は未設定でOK）</param>
	SkillStats(
		short SKILL_POWER,
		const CharacterStats* characterStats = nullptr,
		COLLIDER_TAG COLL_TAG = COLLIDER_TAG::NON
	) :
		SKILL_POWER(SKILL_POWER),
		attackPower(characterStats ? &characterStats->attackPower : nullptr),
		critical(characterStats ? &characterStats->critical : nullptr),
		COLL_TAG(COLL_TAG),
		SKILL_TIME(0)
	{
	}

	/// <summary>
	/// バフ/デバフ スキル生成
	/// </summary>
	/// <param name="SKILL_POWER">技威力</param>
	/// <param name="SKILL_TIME">技効果時間</param>
	SkillStats(short SKILL_POWER, short SKILL_TIME, COLLIDER_TAG COLL_TAG = COLLIDER_TAG::NON) :
		SKILL_POWER(SKILL_POWER), SKILL_TIME(SKILL_TIME),
		COLL_TAG(COLL_TAG),
		attackPower(nullptr), critical(nullptr)
	{
	}
};