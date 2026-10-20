#pragma once

#include <vector>

#include "MeshCutter.h"

class CuttableActor;

// 切断の要求1件
struct CutRequest
{
	// 切る対象
	CuttableActor* target;

	// ワールド座標の切断面
	CutPlane plane;

	// 一振りごとに発行される斬撃ID
	int cutId;

	CutRequest(void) : target(nullptr), plane(), cutId(-1) {}
};

// 衝突判定中（OnCollision）にアクターを増やすと危険なため、
// 切断はいったんここへ積んで、判定が終わった後にシーンがまとめて処理する
class CutRequestQueue
{
public:

	// 新しい斬撃IDを発行する（一振りごとに1回呼ぶ）
	static int IssueCutId(void) { return ++Counter(); }

	// 切断の要求を積む（同じ斬撃IDで同じ対象の要求は1回だけ積む）
	static void Push(CuttableActor* target, const CutPlane& plane, int cutId)
	{
		if (target == nullptr) { return; }

		std::vector<CutRequest>& requests = Storage();

		for (const CutRequest& request : requests) {
			if (request.target == target && request.cutId == cutId) { return; }
		}

		CutRequest request;
		request.target = target;
		request.plane = plane;
		request.cutId = cutId;
		requests.push_back(request);
	}

	// 積まれた要求をすべて取り出し、キューを空にする
	static std::vector<CutRequest> Drain(void)
	{
		std::vector<CutRequest> result;
		result.swap(Storage());
		return result;
	}

	// 要求を捨てる（シーンの開始・終了時に呼ぶ）
	static void Clear(void) { Storage().clear(); }

private:

	CutRequestQueue(void) = delete;

	static std::vector<CutRequest>& Storage(void) { static std::vector<CutRequest> storage; return storage; }

	static int& Counter(void) { static int counter = 0; return counter; }
};
