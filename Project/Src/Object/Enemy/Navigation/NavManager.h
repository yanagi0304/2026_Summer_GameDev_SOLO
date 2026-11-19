#pragma once

#include <deque>
#include <functional>
#include <vector>

#include "AStar.h"
#include "NavGrid.h"

// 経路探索の依頼を順番待ちにして、1フレームに処理する件数を制限する
// 敵が増えても、探索が1フレームに集中して処理落ちしないようにするための窓口
//
// 使い方:
//   敵: RequestPath(自分, 開始, 目標, 結果を受け取る関数) を呼ぶ。結果は後のフレームに届く
//   敵が消える時: Cancel(自分) を必ず呼ぶ(消えた敵に結果を渡さないため)
//   シーン: 毎フレームUpdate()を呼ぶ
class NavManager
{
public:

	// 探索結果を受け取る関数。found=falseなら経路なし
	using Callback = std::function<void(bool found, std::vector<PathPoint>& path)>;

	explicit NavManager(const NavGrid& grid, int budgetPerFrame = 2);

	~NavManager(void) = default;

	// 同じ依頼主の未処理の依頼があれば、その場で新しい内容に置き換える(列の順番は変わらない)
	void RequestPath(const void* owner, const Vector3& start, const Vector3& goal, Callback callback);

	// 依頼主の未処理の依頼を取り消す
	void Cancel(const void* owner);

	// 順番待ちの先頭から、1フレームの上限まで探索して結果を返す
	void Update(void);

	// 1フレームに処理する探索の最大件数
	void SetBudget(int value) { budgetPerFrame = (value > 0) ? value : 1; }

	const NavGrid& GetGrid(void)const { return grid; }

	// 順番待ちの件数 / 直近のUpdateで探索にかかった時間(ミリ秒) / 直近のUpdateで処理した件数
	int GetPendingCount(void)const { return static_cast<int>(queue.size()); }
	double GetLastUpdateMs(void)const { return lastUpdateMs; }
	int GetLastProcessedCount(void)const { return lastProcessed; }

private:

	struct Request
	{
		const void* owner;
		Vector3 start;
		Vector3 goal;
		Callback callback;
	};

	const NavGrid& grid;
	AStar astar;
	std::deque<Request> queue;
	int budgetPerFrame;

	double lastUpdateMs = 0.0;
	int lastProcessed = 0;
};
