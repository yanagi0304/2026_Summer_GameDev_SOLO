#pragma once

#include <vector>
#include <functional>

class CharactorStateBase
{
public:
	CharactorStateBase(
		const std::function<void(void)>& ownChangeState,
		const std::function<bool(void)>& isOwnState
	) :
		OwnChangeState(ownChangeState),
		IsOwnState(isOwnState)
	{
	}
	virtual ~CharactorStateBase() = default;

	// 自分の状態に遷移する条件関数
	virtual void OwnStateConditionUpdate(void) {}

	// 他の状態に遷移する条件関数を追加する関数
	void AddOtherStateCondition(const std::function<void(void)>& func) {
		OtherStateConditions.push_back(func);
	}

	// 他の状態に遷移する条件関数をすべて呼び出す関数
	void OtherStateConditionsUpdate(void) {
		for (std::function<void(void)>& func : OtherStateConditions) { func(); }
		if (!IsOwnState()) { Exit(); }
	}

	// 状態遷移後1度行う初期化処理
	virtual void Enter(void) {}
	// 更新処理
	virtual void Update(void) {}
	// 状態遷移前1度行う終了処理
	virtual void Exit(void) {}

	// 状態関係なく呼び出す処理
	virtual void AlwaysUpdate(void) {}

protected:
	// 自分の状態に遷移させる関数のポインタ
	const std::function<void(void)> OwnChangeState;

	// 現在自分の状態かどうかを返す関数のポインタ
	const std::function<bool(void)> IsOwnState;
private:

	// 他の状態に遷移する条件関数のポインタを格納するベクター
	std::vector<std::function<void(void)>> OtherStateConditions;
};