#include "NavManager.h"

#include <algorithm>
#include <chrono>
#include <utility>

NavManager::NavManager(const NavGrid& grid, int budgetPerFrame) :
	grid(grid),
	astar(),
	queue(),
	budgetPerFrame((budgetPerFrame > 0) ? budgetPerFrame : 1)
{
}

void NavManager::RequestPath(const void* owner, const Vector3& start, const Vector3& goal, Callback callback)
{
	// 同じ依頼主の依頼があれば置き換える(古い目標のまま探索するのを避ける)
	for (Request& r : queue)
	{
		if (r.owner == owner)
		{
			r.start = start;
			r.goal = goal;
			r.callback = std::move(callback);
			return;
		}
	}

	queue.push_back({ owner, start, goal, std::move(callback) });
}

void NavManager::Cancel(const void* owner)
{
	queue.erase(
		std::remove_if(queue.begin(), queue.end(), [owner](const Request& r) { return r.owner == owner; }),
		queue.end());
}

void NavManager::Update(void)
{
	using Clock = std::chrono::high_resolution_clock;
	const auto begin = Clock::now();

	lastProcessed = 0;

	while (!queue.empty() && lastProcessed < budgetPerFrame)
	{
		// 先に取り出してから呼ぶ(結果を受け取る側が、その場で新しい依頼を出しても安全にする)
		Request request = std::move(queue.front());
		queue.pop_front();

		std::vector<PathPoint> path;
		const bool found = astar.FindPath(grid, request.start, request.goal, path, options);
		if (request.callback) { request.callback(found, path); }

		++lastProcessed;
	}

	lastUpdateMs = std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}
