#pragma once

#include <cstdint>
#include <vector>

#include "NavGrid.h"

// 経路上の1点で行う動作
// 今は歩くだけ。将来グラスホッパーなどで移動する点を追加できるようにしてある
enum class PathAction
{
	Walk,
};

// 経路上の1点
struct PathPoint
{
	Vector3 pos;
	PathAction action = PathAction::Walk;
};

// グリッド上のA*探索(8方向)
// 作業用バッファを使い回すため、インスタンスを1つ作って使い続けること
// (複数スレッドから同時には使えない)
class AStar
{
public:

	// 探索設定
	struct Options
	{
		// 開始・目標がセル外/壁の中の時に、最寄りの歩行可能セルを探す範囲(セル数)
		int snapRadius = 8;

		// 展開するノード数の上限(広いマップで1回の探索が重くなりすぎないように)
		int maxExpandedNodes = 200000;

		// ヒューリスティックの重み。1.0で最短経路。大きくすると、探索するセルが減って速くなる代わりに、
		// 経路が最短よりやや遠回りになることがある(1.5なら最大で1.5倍まで)。敵の追跡には十分
		float heuristicWeight = 1.0f;

		// 経路上の不要な中間点を、直線で歩ける限り取り除く
		bool smooth = true;
	};

	AStar(void) = default;
	~AStar(void) = default;

	// 探索。成功したらtrueを返し、outPathに「開始位置の次の点」から目標位置までの経路を入れる
	bool FindPath(const NavGrid& grid, const Vector3& start, const Vector3& goal,
		std::vector<PathPoint>& outPath, const Options& options);
	bool FindPath(const NavGrid& grid, const Vector3& start, const Vector3& goal,
		std::vector<PathPoint>& outPath)
	{
		return FindPath(grid, start, goal, outPath, Options());
	}

private:

	// 作業用バッファを探索ごとに初期化しなくて済むよう、世代番号で有効かを判断する
	void PrepareBuffers(size_t cellCount);

	std::vector<float> gCost;
	std::vector<int> parent;
	std::vector<uint32_t> openStamp;
	std::vector<uint32_t> closedStamp;
	uint32_t stamp = 0;
};
