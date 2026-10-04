#pragma once

#include "../Common/ActorBase/ActorBase.h"

#include "../Weapon/TriggerTypeDefine.h"

class TriggerUI : public ActorBase
{
public:

	const float SIZE_X = 499;
	const float SIZE_Y = 325;

	static constexpr float OFFSET = 20.0f;

public:

	TriggerUI(const int& mainCurrentIndex_, const int& subCurrentIndex_);
	~TriggerUI() = default;

	void Load();
	const char* TriggerToString(TriggerType trigger);
	void AddMain(TriggerType trigger) {mainTrig_.push_back(trigger); }
	void AddSub(TriggerType trigger) { subTrig_.push_back(trigger); }
	void Setweapon(const TriggerType trigger) { currentTrig_ = trigger; }
	std::vector<TriggerType> GetMainTrigger(void) const { return mainTrig_; }

private:

	std::vector<TriggerType> mainTrig_;
	std::vector<TriggerType> subTrig_;
	std::map< TriggerType, int> icon_;	//武器アイコン
	TriggerType currentTrig_;

	//画像ハンドル
	int triggerUI_;
	int select_;

	int x, y = 0;

	void SubUpdate();
	void SubDraw();

	// 現在の選択状態の参照（メイントリガー）
	const int& mainCurrentIndex_;
	// 現在の選択状態の参照（サブトリガー）
	const int& subCurrentIndex_;
};

