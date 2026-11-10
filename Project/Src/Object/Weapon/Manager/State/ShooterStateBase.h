#pragma once

// 撃つ武器の状態(State:ステート=状態)
enum class SHOOTER_STATE
{
	// 何も出していない(Idle:アイドル=待機)
	Idle,

	// 手元にアステロイドが1個ある(Cube:キューブ=立方体)
	Cube,

	// 27個に分割して並べている(Split:スプリット=分割)
	Split,

	// 発射中(Fire:ファイア=撃つ)
	Fire,

	Max,
};

// 撃つ武器の状態の基底クラス
// 「入ったとき・更新・出るとき・次の状態・アニメ」をこの状態の中だけで完結させる
class ShooterStateBase
{
public:

	ShooterStateBase(void) = default;

	virtual ~ShooterStateBase() = default;

	// 状態遷移後に1度行う初期化処理(Enter:エンター=入る)
	virtual void Enter(void) {}

	// 更新処理
	virtual void Update(void) {}

	// 状態遷移前に1度行う終了処理(Exit:エグジット=出る)
	virtual void Exit(void) {}

	// 次の状態(ボタンを押したときの遷移先)
	virtual SHOOTER_STATE NextState(void) const = 0;

	// この状態で再生する待機アニメーションのID
	virtual int GetIdleAnimeID(void) const = 0;
};
