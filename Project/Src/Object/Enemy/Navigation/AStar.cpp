#include "AStar.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>

namespace
{
	// 斜め移動のコスト(直進を1とする)
	constexpr float DIAGONAL_COST = 1.41421356f;

	// 8方向(直進4 + 斜め4)
	constexpr int DIR_X[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };
	constexpr int DIR_Y[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };

	// オクタイル距離(8方向移動での最短コストの見積もり)
	float Heuristic(int ax, int ay, int bx, int by)
	{
		const float dx = static_cast<float>(std::abs(ax - bx));
		const float dy = static_cast<float>(std::abs(ay - by));
		return (dx + dy) + (DIAGONAL_COST - 2.0f) * (std::min)(dx, dy);
	}
}

void AStar::PrepareBuffers(size_t cellCount)
{
	if (gCost.size() != cellCount)
	{
		gCost.assign(cellCount, 0.0f);
		parent.assign(cellCount, -1);
		openStamp.assign(cellCount, 0);
		closedStamp.assign(cellCount, 0);
		stamp = 0;
	}

	++stamp;

	// 世代番号が一周したら全部クリアする
	if (stamp == 0)
	{
		std::fill(openStamp.begin(), openStamp.end(), 0);
		std::fill(closedStamp.begin(), closedStamp.end(), 0);
		stamp = 1;
	}
}

bool AStar::FindPath(const NavGrid& grid, const Vector3& start, const Vector3& goal,
	std::vector<PathPoint>& outPath, const Options& options)
{
	outPath.clear();

	if (!grid.IsBuilt()) { return false; }

	// 開始・目標が壁の中などでも、最寄りの歩ける場所に補正して探索する
	const Vector2I rawStart = grid.WorldToCell(start);
	const Vector2I rawGoal = grid.WorldToCell(goal);
	const auto startCell = grid.FindNearestWalkable(rawStart, options.snapRadius);
	const auto goalCell = grid.FindNearestWalkable(rawGoal, options.snapRadius);
	if (!startCell || !goalCell) { return false; }

	const int width = grid.GetWidth();
	const int height = grid.GetHeight();
	PrepareBuffers(static_cast<size_t>(width) * height);

	const int startIdx = startCell->y * width + startCell->x;
	const int goalIdx = goalCell->y * width + goalCell->x;

	// 目標の位置: 目標が歩けるセルならその座標のまま、補正した場合はセル中心
	Vector3 goalPos = grid.CellToWorld(*goalCell);
	if (*goalCell == rawGoal)
	{
		goalPos.x = goal.x;
		goalPos.z = goal.z;
	}

	std::vector<Vector3> points;
	points.push_back(start);

	if (startIdx == goalIdx)
	{
		points.push_back(goalPos);
	}
	else
	{
		// ---- A*本体 ----
		using Entry = std::pair<float, int>;
		std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;

		gCost[startIdx] = 0.0f;
		parent[startIdx] = -1;
		openStamp[startIdx] = stamp;
		open.emplace(Heuristic(startCell->x, startCell->y, goalCell->x, goalCell->y), startIdx);

		bool found = false;
		int expanded = 0;

		while (!open.empty())
		{
			const int current = open.top().second;
			open.pop();

			// 同じセルが複数回積まれることがあるので、処理済みは飛ばす
			if (closedStamp[current] == stamp) { continue; }
			closedStamp[current] = stamp;

			if (current == goalIdx)
			{
				found = true;
				break;
			}

			if (++expanded > options.maxExpandedNodes) { break; }

			const int cx = current % width;
			const int cy = current / width;

			for (int d = 0; d < 8; ++d)
			{
				const int nx = cx + DIR_X[d];
				const int ny = cy + DIR_Y[d];
				if (!grid.IsWalkable(nx, ny)) { continue; }

				// 斜め移動は、両隣が歩けるときだけ(壁の角を抜けないように)
				const bool diagonal = (DIR_X[d] != 0 && DIR_Y[d] != 0);
				if (diagonal && (!grid.IsWalkable(cx + DIR_X[d], cy) || !grid.IsWalkable(cx, cy + DIR_Y[d])))
				{
					continue;
				}

				const int next = ny * width + nx;
				if (closedStamp[next] == stamp) { continue; }

				const float newCost = gCost[current] + (diagonal ? DIAGONAL_COST : 1.0f);
				if (openStamp[next] != stamp || newCost < gCost[next])
				{
					openStamp[next] = stamp;
					gCost[next] = newCost;
					parent[next] = current;
					open.emplace(newCost + Heuristic(nx, ny, goalCell->x, goalCell->y), next);
				}
			}
		}

		if (!found) { return false; }

		// ---- 経路復元(目標 → 開始) ----
		std::vector<int> cells;
		for (int i = goalIdx; i != -1; i = parent[i])
		{
			cells.push_back(i);
		}
		std::reverse(cells.begin(), cells.end());

		// 先頭(開始セル)と末尾(目標セル)の中心は、実際の位置で置き換えるので入れない
		for (size_t i = 1; i + 1 < cells.size(); ++i)
		{
			points.push_back(grid.CellToWorld(Vector2I(cells[i] % width, cells[i] / width)));
		}
		points.push_back(goalPos);
	}

	// ---- スムージング ----
	// 今の点から直線で歩ける一番遠い点へ直接向かい、間の点を捨てる
	if (options.smooth)
	{
		size_t i = 0;
		while (i + 1 < points.size())
		{
			size_t j = points.size() - 1;
			while (j > i + 1 && !grid.IsLineWalkable(points[i], points[j]))
			{
				--j;
			}
			outPath.push_back({ points[j], PathAction::Walk });
			i = j;
		}
	}
	else
	{
		for (size_t i = 1; i < points.size(); ++i)
		{
			outPath.push_back({ points[i], PathAction::Walk });
		}
	}

	return true;
}
