#include "MeshCutter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
	// 面上とみなす距離（これ以内の頂点は「どちら側にも属する」扱い）
	constexpr float CUT_EPSILON = 0.001f;

	// 三角形を平面でクリップしたときの最大頂点数（実際は最大4）
	constexpr int POLY_MAX = 6;

	// クリップ後の多角形の頂点
	// 元の頂点そのもの、または「辺と切断面の交点」のどちらか
	struct PolyVert
	{
		// true = 交点、false = 元の頂点
		bool isCut;

		// 元の頂点番号（交点のときは辺の始点側）
		unsigned int a;

		// 交点のときの辺の終点側
		unsigned int b;

		// 交点のときの a, b それぞれの平面までの符号付き距離
		float da, db;
	};

	// 2頂点の補間（座標・法線・UV・カラー）
	CutVertex LerpVertex(const CutVertex& a, const CutVertex& b, float t)
	{
		CutVertex r;

		r.pos = a.pos + (b.pos - a.pos) * t;
		r.normal = (a.normal + (b.normal - a.normal) * t).Normalized();
		r.u = a.u + (b.u - a.u) * t;
		r.v = a.v + (b.v - a.v) * t;
		r.color.r = a.color.r + (b.color.r - a.color.r) * t;
		r.color.g = a.color.g + (b.color.g - a.color.g) * t;
		r.color.b = a.color.b + (b.color.b - a.color.b) * t;
		r.color.a = a.color.a + (b.color.a - a.color.a) * t;

		return r;
	}

	// 切断結果の片側を組み立てるクラス
	class SideBuilder
	{
	public:

		SideBuilder(const CutMesh& src, CutMesh& out) :
			src(src),
			out(out),
			originalMap(src.vertices.size(), -1),
			cutMap()
		{
			out.Clear();
		}

		// 多角形を三角形ファンにして出力へ追加する
		void AddPolygon(int materialIndex, const PolyVert* poly, int num)
		{
			// 3頂点未満なら面にならないので捨てる
			if (num < 3) { return; }

			CutSubMesh& sub = GetSubMesh(materialIndex);

			const unsigned int first = Resolve(poly[0]);
			for (int i = 1; i + 1 < num; i++) {
				const unsigned int second = Resolve(poly[i]);
				const unsigned int third = Resolve(poly[i + 1]);
				sub.indices.push_back(first);
				sub.indices.push_back(second);
				sub.indices.push_back(third);
			}
		}

	private:

		const CutMesh& src;
		CutMesh& out;

		// 元の頂点番号 -> 出力側の頂点番号（-1 = まだ無い）
		std::vector<int> originalMap;

		// 辺（小さい番号, 大きい番号）-> 交点の頂点番号
		// 隣り合う三角形で同じ交点を共有するために使う
		std::unordered_map<std::uint64_t, unsigned int> cutMap;

		unsigned int Resolve(const PolyVert& p)
		{
			return p.isCut ? Intersect(p) : Original(p.a);
		}

		// 元の頂点を出力へコピー（2回目以降は同じ番号を返す）
		unsigned int Original(unsigned int srcIndex)
		{
			int& mapped = originalMap[srcIndex];
			if (mapped < 0) {
				mapped = static_cast<int>(out.vertices.size());
				out.vertices.push_back(src.vertices[srcIndex]);
			}
			return static_cast<unsigned int>(mapped);
		}

		// 辺と平面の交点を出力へ追加（同じ辺なら同じ番号を返す）
		unsigned int Intersect(const PolyVert& p)
		{
			unsigned int lo = p.a;
			unsigned int hi = p.b;
			float dLo = p.da;
			float dHi = p.db;

			// 向きによらず同じ結果になるよう、番号の小さい方を始点にそろえる
			if (lo > hi) {
				std::swap(lo, hi);
				std::swap(dLo, dHi);
			}

			const std::uint64_t key = (static_cast<std::uint64_t>(lo) << 32) | hi;
			const auto it = cutMap.find(key);
			if (it != cutMap.end()) {
				return it->second;
			}

			// dLo と dHi は逆符号なので、分母が0になることはない
			const float t = dLo / (dLo - dHi);

			const unsigned int index = static_cast<unsigned int>(out.vertices.size());
			out.vertices.push_back(LerpVertex(src.vertices[lo], src.vertices[hi], t));
			cutMap[key] = index;

			return index;
		}

		// マテリアル番号に対応するサブメッシュを取得（無ければ作る）
		CutSubMesh& GetSubMesh(int materialIndex)
		{
			for (CutSubMesh& sub : out.subMeshes) {
				if (sub.materialIndex == materialIndex) { return sub; }
			}
			out.subMeshes.emplace_back(materialIndex);
			return out.subMeshes.back();
		}
	};

#pragma region 切り口のふた

	// ふた専用のマテリアル番号（テクスチャなし）
	constexpr int CAP_MATERIAL_INDEX = -1;

	// ふたの頂点カラー（テクスチャが無いので、これがそのまま色になる）
	constexpr float CAP_COLOR_R = 0.9f;
	constexpr float CAP_COLOR_G = 0.9f;
	constexpr float CAP_COLOR_B = 0.9f;

	// ふたのUV（座標にこの値をかけたもの）
	constexpr float CAP_UV_SCALE = 0.01f;

	// 耳切り法で処理する輪の最大頂点数（これを超えたらファンで簡易的に埋める）
	// （耳切り法は頂点数の3乗に近い時間がかかるので、重くならない範囲に抑える）
	constexpr size_t EAR_CLIP_LIMIT = 400;

	// 切断面の上にできる線分（向きは「ふたの外周が法線まわりに左回り」になるよう揃える）
	struct CapSegment
	{
		Vector3 a;
		Vector3 b;
	};

	// 近い点を同じ点としてまとめる（元のモデルは同じ位置でも頂点が分かれていることがある）
	class PointWelder
	{
	public:

		explicit PointWelder(float tolerance) :
			tolerance(tolerance),
			points(),
			grid()
		{
		}

		// 既にある近い点の番号、無ければ新しく登録した番号を返す
		int Find(const Vector3& p)
		{
			const int cx = Cell(p.x);
			const int cy = Cell(p.y);
			const int cz = Cell(p.z);

			const float toleranceSq = tolerance * tolerance;

			for (int dx = -1; dx <= 1; dx++) {
				for (int dy = -1; dy <= 1; dy++) {
					for (int dz = -1; dz <= 1; dz++) {

						const auto it = grid.find(Hash(cx + dx, cy + dy, cz + dz));
						if (it == grid.end()) { continue; }

						for (int id : it->second) {
							if ((points[id] - p).LengthSq() <= toleranceSq) { return id; }
						}
					}
				}
			}

			const int id = static_cast<int>(points.size());
			points.push_back(p);
			grid[Hash(cx, cy, cz)].push_back(id);

			return id;
		}

		const Vector3& Get(int id) const { return points[id]; }

	private:

		float tolerance;

		std::vector<Vector3> points;

		// セル -> そのセルにある点の番号
		// ハッシュが衝突しても、距離で必ず確かめるので結果は変わらない
		std::unordered_map<std::uint64_t, std::vector<int>> grid;

		int Cell(float value) const { return static_cast<int>(std::floor(value / tolerance)); }

		static std::uint64_t Hash(int x, int y, int z)
		{
			return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) * 73856093ULL) ^
				(static_cast<std::uint64_t>(static_cast<std::uint32_t>(y)) * 19349663ULL) ^
				(static_cast<std::uint64_t>(static_cast<std::uint32_t>(z)) * 83492791ULL);
		}
	};

	// 辺と平面の交点（SideBuilder::Intersectと同じく、番号の小さい頂点を始点にそろえて求める）
	Vector3 EdgePlanePoint(const CutMesh& src, unsigned int a, unsigned int b, float da, float db)
	{
		unsigned int lo = a;
		unsigned int hi = b;
		float dLo = da;
		float dHi = db;

		if (lo > hi) {
			std::swap(lo, hi);
			std::swap(dLo, dHi);
		}

		const float t = dLo / (dLo - dHi);

		const Vector3& pLo = src.vertices[lo].pos;
		const Vector3& pHi = src.vertices[hi].pos;

		return pLo + (pHi - pLo) * t;
	}

	// 三角形が平面をまたいでいるとき、その三角形と平面の交わりの線分を集める
	void CollectCapSegment(
		const CutMesh& src,
		const unsigned int idx[3],
		const float d[3],
		const CutPlane& plane,
		std::vector<CapSegment>& segments)
	{
		int positiveNum = 0;
		int negativeNum = 0;

		for (int k = 0; k < 3; k++) {
			if (d[k] > CUT_EPSILON) { positiveNum++; }
			else if (d[k] < -CUT_EPSILON) { negativeNum++; }
		}

		// 面上の頂点の数
		const int onPlaneNum = 3 - positiveNum - negativeNum;

		// 片側にしか無い三角形（面に触れるだけ）は線分を作らない
		// ただし2頂点が面上にある三角形は、その辺が切り口の一部になる（平面が辺や頂点をちょうど通るとき）
		if ((positiveNum == 0 || negativeNum == 0) && onPlaneNum != 2) { return; }

		Vector3 points[3];
		int pointNum = 0;

		for (int k = 0; k < 3; k++) {

			const int next = (k + 1) % 3;

			// 面上の頂点
			if (std::fabs(d[k]) <= CUT_EPSILON) { points[pointNum++] = src.vertices[idx[k]].pos; }

			// 平面をまたぐ辺の交点
			const bool cross = (d[k] > CUT_EPSILON && d[next] < -CUT_EPSILON) || (d[k] < -CUT_EPSILON && d[next] > CUT_EPSILON);
			if (cross) { points[pointNum++] = EdgePlanePoint(src, idx[k], idx[next], d[k], d[next]); }
		}

		if (pointNum != 2) { return; }

		// 三角形の外向きの法線（このエンジンでは、表の面の外積が外向き）
		const Vector3 p0 = src.vertices[idx[0]].pos;
		const Vector3 p1 = src.vertices[idx[1]].pos;
		const Vector3 p2 = src.vertices[idx[2]].pos;
		const Vector3 triangleNormal = (p1 - p0).Cross(p2 - p0);

		// 立体の内側が左に来る向き（外向きの法線 = 進行方向 x 平面の法線）に揃える
		const Vector3 direction = plane.normal.Cross(triangleNormal);

		CapSegment segment;
		segment.a = points[0];
		segment.b = points[1];

		if ((segment.b - segment.a).Dot(direction) < 0.0f) { std::swap(segment.a, segment.b); }

		segments.push_back(segment);
	}

	// 輪（3D座標の点の並び）を耳切り法で三角形に分ける
	// loop は法線まわりに左回りの輪。結果は loop の添字3つ1組で outTriangles へ追加する
	void TriangulateLoop(
		const std::vector<Vector3>& loop,
		const Vector3& normal,
		float areaMin,
		std::vector<unsigned int>& outTriangles)
	{
		const size_t n = loop.size();
		if (n < 3) { return; }

		// 平面上の2軸（u x v = normal になるようにとる）
		const Vector3 ref = (std::fabs(normal.x) < 0.9f) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
		const Vector3 axisU = (ref - normal * ref.Dot(normal)).Normalized();
		const Vector3 axisV = normal.Cross(axisU);

		std::vector<float> xs(n);
		std::vector<float> ys(n);

		for (size_t i = 0; i < n; i++) {
			xs[i] = loop[i].Dot(axisU);
			ys[i] = loop[i].Dot(axisV);
		}

		// 符号付き面積（左回りが正）。負は穴の輪なので、ここでは埋めない
		float area = 0.0f;
		for (size_t i = 0; i < n; i++) {
			const size_t j = (i + 1) % n;
			area += xs[i] * ys[j] - xs[j] * ys[i];
		}
		area *= 0.5f;

		if (area <= areaMin) { return; }

		// 頂点数が多すぎるときは、ファンで簡易的に埋める
		if (n > EAR_CLIP_LIMIT) {
			for (size_t i = 1; i + 1 < n; i++) {
				outTriangles.push_back(0);
				outTriangles.push_back(static_cast<unsigned int>(i));
				outTriangles.push_back(static_cast<unsigned int>(i + 1));
			}
			return;
		}

		std::vector<unsigned int> remain(n);
		for (size_t i = 0; i < n; i++) { remain[i] = static_cast<unsigned int>(i); }

		auto cross2 = [&](unsigned int a, unsigned int b, unsigned int c) {
			return (xs[b] - xs[a]) * (ys[c] - ys[a]) - (ys[b] - ys[a]) * (xs[c] - xs[a]);
		};

		while (remain.size() > 3) {

			const size_t count = remain.size();

			bool found = false;

			// 凸で、中に他の頂点を含まない頂点（耳）を探す
			for (size_t i = 0; i < count; i++) {

				const unsigned int a = remain[(i + count - 1) % count];
				const unsigned int b = remain[i];
				const unsigned int c = remain[(i + 1) % count];

				if (cross2(a, b, c) <= 0.0f) { continue; }

				bool contains = false;

				for (size_t j = 0; j < count; j++) {

					const unsigned int p = remain[j];
					if (p == a || p == b || p == c) { continue; }

					if (cross2(a, b, p) >= 0.0f && cross2(b, c, p) >= 0.0f && cross2(c, a, p) >= 0.0f) {
						contains = true;
						break;
					}
				}

				if (contains) { continue; }

				outTriangles.push_back(a);
				outTriangles.push_back(b);
				outTriangles.push_back(c);

				remain.erase(remain.begin() + i);
				found = true;
				break;
			}

			if (found) { continue; }

			// 耳が見つからない（ほぼ一直線に並ぶ頂点など）ときは、面積が一番小さい頂点を捨てて進める
			size_t smallest = 0;
			float smallestArea = std::fabs(cross2(remain[count - 1], remain[0], remain[1]));

			for (size_t i = 1; i < count; i++) {
				const float a = std::fabs(cross2(remain[(i + count - 1) % count], remain[i], remain[(i + 1) % count]));
				if (a < smallestArea) {
					smallestArea = a;
					smallest = i;
				}
			}

			remain.erase(remain.begin() + smallest);
		}

		if (remain.size() == 3 && cross2(remain[0], remain[1], remain[2]) > 0.0f) {
			outTriangles.push_back(remain[0]);
			outTriangles.push_back(remain[1]);
			outTriangles.push_back(remain[2]);
		}
	}

	// ふたの頂点を作る
	CutVertex MakeCapVertex(const Vector3& pos, const Vector3& normal, const Vector3& origin, const Vector3& axisU, const Vector3& axisV)
	{
		CutVertex v;

		v.pos = pos;
		v.normal = normal;

		v.u = (pos - origin).Dot(axisU) * CAP_UV_SCALE;
		v.v = (pos - origin).Dot(axisV) * CAP_UV_SCALE;

		v.color.r = CAP_COLOR_R;
		v.color.g = CAP_COLOR_G;
		v.color.b = CAP_COLOR_B;
		v.color.a = 1.0f;

		return v;
	}

	// ふた用のサブメッシュを取得（無ければ作る）
	CutSubMesh& GetCapSubMesh(CutMesh& mesh)
	{
		for (CutSubMesh& sub : mesh.subMeshes) {
			if (sub.materialIndex == CAP_MATERIAL_INDEX) { return sub; }
		}
		mesh.subMeshes.emplace_back(CAP_MATERIAL_INDEX);
		return mesh.subMeshes.back();
	}

	// 同じ点を2回通る輪（8の字）を、同じ点のところで切り分けて、単純な輪の集まりにする
	// 平面が凹んだ角の頂点をちょうど通ると、切り口が8の字になることがある
	std::vector<std::vector<int>> SplitAtRepeats(const std::vector<int>& loop)
	{
		std::vector<std::vector<int>> result;

		std::vector<int> stack;
		std::unordered_map<int, size_t> where;

		for (int id : loop) {

			const auto it = where.find(id);

			if (it == where.end()) {
				where[id] = stack.size();
				stack.push_back(id);
				continue;
			}

			// 以前に通った点へ戻ってきた = そこから今までが1つの輪
			const size_t from = it->second;

			std::vector<int> sub(stack.begin() + from, stack.end());

			for (size_t k = from; k < stack.size(); k++) { where.erase(stack[k]); }
			stack.resize(from);

			if (sub.size() >= 3) { result.push_back(sub); }

			// 分岐の点は、残りの輪の続きとして積み直す
			where[id] = stack.size();
			stack.push_back(id);
		}

		if (stack.size() >= 3) { result.push_back(stack); }

		return result;
	}

	// 集めた線分から切り口のふたを作り、表側・裏側の両方へ追加する
	void BuildCaps(
		const CutMesh& src,
		const CutPlane& plane,
		const std::vector<CapSegment>& segments,
		CutMesh& front,
		CutMesh& back)
	{
		if (segments.empty()) { return; }

		// 点をまとめる許容距離（メッシュの大きさに合わせる）
		Vector3 minPos = src.vertices[0].pos;
		Vector3 maxPos = minPos;

		for (const CutVertex& v : src.vertices) {
			minPos.x = (std::min)(minPos.x, v.pos.x);
			minPos.y = (std::min)(minPos.y, v.pos.y);
			minPos.z = (std::min)(minPos.z, v.pos.z);
			maxPos.x = (std::max)(maxPos.x, v.pos.x);
			maxPos.y = (std::max)(maxPos.y, v.pos.y);
			maxPos.z = (std::max)(maxPos.z, v.pos.z);
		}

		const float tolerance = (std::max)(0.0001f, (maxPos - minPos).Length() * 0.00001f);

		// 線分の端点を、まとめた点の番号に置き換える
		struct Edge
		{
			int from;
			int to;
			bool used;
		};

		PointWelder welder(tolerance);

		std::vector<Edge> edges;
		edges.reserve(segments.size());

		for (const CapSegment& s : segments) {

			const int from = welder.Find(s.a);
			const int to = welder.Find(s.b);

			// 点がつぶれた線分は捨てる
			if (from == to) { continue; }

			edges.push_back({ from, to, false });
		}

		// 同じ線分の重複と、行って戻るだけの往復の線分を整理する
		// （平面が辺や頂点をちょうど通ると、隣り合う2枚の三角形から同じ辺が出てくることがある）
		{
			std::unordered_map<std::uint64_t, int> directed;

			auto edgeKey = [](int from, int to) {
				return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(from)) << 32) | static_cast<std::uint32_t>(to);
			};

			for (const Edge& e : edges) { directed[edgeKey(e.from, e.to)]++; }

			std::vector<Edge> cleaned;
			cleaned.reserve(edges.size());

			for (const Edge& e : edges) {

				const auto self = directed.find(edgeKey(e.from, e.to));
				if (self == directed.end() || self->second <= 0) { continue; }

				// 逆向きの線分があれば、往復なのでどちらも捨てる
				if (directed.find(edgeKey(e.to, e.from)) != directed.end()) { continue; }

				// 重複は1本にまとめる
				self->second = 0;

				cleaned.push_back(e);
			}

			edges.swap(cleaned);
		}

		// 点 -> そこから出る線分
		std::unordered_map<int, std::vector<int>> outgoing;

		for (size_t i = 0; i < edges.size(); i++) {
			outgoing[edges[i].from].push_back(static_cast<int>(i));
		}

		// 線分をつないで、閉じた輪を作る（途中で途切れたものは捨てる）
		std::vector<std::vector<int>> loops;

		for (size_t first = 0; first < edges.size(); first++) {

			if (edges[first].used) { continue; }

			const int startPoint = edges[first].from;

			std::vector<int> loop;
			int current = static_cast<int>(first);
			bool closed = false;

			while (true) {

				edges[current].used = true;
				loop.push_back(edges[current].from);

				const int toPoint = edges[current].to;

				if (toPoint == startPoint) {
					closed = true;
					break;
				}

				int next = -1;

				const auto it = outgoing.find(toPoint);
				if (it != outgoing.end()) {
					for (int candidate : it->second) {
						if (!edges[candidate].used) {
							next = candidate;
							break;
						}
					}
				}

				if (next < 0) { break; }

				current = next;
			}

			if (closed && loop.size() >= 3) {
				for (std::vector<int>& simple : SplitAtRepeats(loop)) { loops.push_back(std::move(simple)); }
			}
		}

		if (loops.empty()) { return; }

		// ふたのUV用の座標系（原点は切断面上の点）
		const Vector3 ref = (std::fabs(plane.normal.x) < 0.9f) ? Vector3(1.0f, 0.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
		const Vector3 axisU = (ref - plane.normal * ref.Dot(plane.normal)).Normalized();
		const Vector3 axisV = plane.normal.Cross(axisU);

		const Vector3 frontNormal = plane.normal * -1.0f;	// 表側のふたは、切り落とした裏側を向く
		const Vector3 backNormal = plane.normal;			// 裏側のふたは、表側を向く

		const float areaMin = tolerance * tolerance;

		CutSubMesh& frontSub = GetCapSubMesh(front);
		CutSubMesh& backSub = GetCapSubMesh(back);

		for (const std::vector<int>& loopIds : loops) {

			std::vector<Vector3> loopPoints;
			loopPoints.reserve(loopIds.size());
			for (int id : loopIds) { loopPoints.push_back(welder.Get(id)); }

			std::vector<unsigned int> triangles;
			TriangulateLoop(loopPoints, plane.normal, areaMin, triangles);

			if (triangles.empty()) { continue; }

			// 輪の頂点を、表側・裏側それぞれへ追加する
			const unsigned int frontBase = static_cast<unsigned int>(front.vertices.size());
			const unsigned int backBase = static_cast<unsigned int>(back.vertices.size());

			for (const Vector3& p : loopPoints) {
				front.vertices.push_back(MakeCapVertex(p, frontNormal, plane.point, axisU, axisV));
				back.vertices.push_back(MakeCapVertex(p, backNormal, plane.point, axisU, axisV));
			}

			for (size_t i = 0; i + 2 < triangles.size(); i += 3) {

				const unsigned int a = triangles[i];
				const unsigned int b = triangles[i + 1];
				const unsigned int c = triangles[i + 2];

				// 三角形の外積が外向きになるよう、裏側は左回り（法線向き）、表側は逆回りにする
				backSub.indices.push_back(backBase + a);
				backSub.indices.push_back(backBase + b);
				backSub.indices.push_back(backBase + c);

				frontSub.indices.push_back(frontBase + a);
				frontSub.indices.push_back(frontBase + c);
				frontSub.indices.push_back(frontBase + b);
			}
		}
	}

#pragma endregion
}

bool MeshCutter::Cut(const CutMesh& src, const CutPlane& plane, CutMesh& front, CutMesh& back)
{
	SideBuilder frontBuilder(src, front);
	SideBuilder backBuilder(src, back);

	// 法線が0の平面では切れない
	if (plane.normal.LengthSq() < 0.5f) {
		return false;
	}

	// 切り口にできる線分（ふたを作るために集める）
	std::vector<CapSegment> capSegments;

	for (const CutSubMesh& sub : src.subMeshes) {
		for (size_t i = 0; i + 2 < sub.indices.size(); i += 3) {

			const unsigned int idx[3] = { sub.indices[i], sub.indices[i + 1], sub.indices[i + 2] };

			float d[3];
			for (int k = 0; k < 3; k++) {
				d[k] = plane.Distance(src.vertices[idx[k]].pos);
			}

			// 3頂点とも面上にある三角形は、二重にならないよう表側だけに入れる
			if (std::fabs(d[0]) <= CUT_EPSILON && std::fabs(d[1]) <= CUT_EPSILON && std::fabs(d[2]) <= CUT_EPSILON) {
				const PolyVert tri[3] = {
					{ false, idx[0], 0u, 0.0f, 0.0f },
					{ false, idx[1], 0u, 0.0f, 0.0f },
					{ false, idx[2], 0u, 0.0f, 0.0f },
				};
				frontBuilder.AddPolygon(sub.materialIndex, tri, 3);
				continue;
			}

			// 切り口の線分を集める
			CollectCapSegment(src, idx, d, plane, capSegments);

			// 三角形を平面でクリップして、表側と裏側の多角形を作る（Sutherland-Hodgman法）
			PolyVert frontPoly[POLY_MAX];
			PolyVert backPoly[POLY_MAX];
			int frontNum = 0;
			int backNum = 0;

			for (int k = 0; k < 3; k++) {

				const int next = (k + 1) % 3;
				const float da = d[k];
				const float db = d[next];

				// 頂点そのものの振り分け（面上の頂点は両側へ入れる）
				const PolyVert own = { false, idx[k], 0u, 0.0f, 0.0f };
				if (da >= -CUT_EPSILON) { frontPoly[frontNum++] = own; }
				if (da <= CUT_EPSILON) { backPoly[backNum++] = own; }

				// 辺が平面をまたぐなら、交点を両側へ入れる
				const bool cross = (da > CUT_EPSILON && db < -CUT_EPSILON) || (da < -CUT_EPSILON && db > CUT_EPSILON);
				if (cross) {
					const PolyVert cut = { true, idx[k], idx[next], da, db };
					frontPoly[frontNum++] = cut;
					backPoly[backNum++] = cut;
				}
			}

			frontBuilder.AddPolygon(sub.materialIndex, frontPoly, frontNum);
			backBuilder.AddPolygon(sub.materialIndex, backPoly, backNum);
		}
	}

	// 片側にしか三角形が無ければ、切れていない
	if (front.IsEmpty() || back.IsEmpty()) { return false; }

	// 切り口にふたをする
	BuildCaps(src, plane, capSegments, front, back);

	return true;
}
