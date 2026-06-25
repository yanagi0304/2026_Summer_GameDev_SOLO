#pragma once
#include <string>
#include <vector>
#include <map>

class TriggerUI
{
public:
	enum class TriggerType
	{
		KOGETSU,
		ASTEROID,
		GRASS_HOPPER,
		HOUND,
		VIPER,
		SHIELD,
	};

public:

	const float SIZE_X = 499;
	const float SIZE_Y = 325;

	static constexpr float OFFSET = 20.0f;

public:



	TriggerUI();
	~TriggerUI() = default;

	void Load();
	void Update();
	void Draw(int index);
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

};

