#include "NavGrid.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace
{
	// 点と線分の距離(XZ平面)
	float DistancePointSegment(float px, float pz, float ax, float az, float bx, float bz)
	{
		const float abx = bx - ax;
		const float abz = bz - az;
		const float lenSq = abx * abx + abz * abz;
		float t = 0.0f;
		if (lenSq > 1.0e-9f)
		{
			t = ((px - ax) * abx + (pz - az) * abz) / lenSq;
			t = (std::max)(0.0f, (std::min)(1.0f, t));
		}
		const float cx = ax + abx * t - px;
		const float cz = az + abz * t - pz;
		return std::sqrt(cx * cx + cz * cz);
	}

	// 点と三角形の距離(XZ平面。三角形の内側なら0。つぶれた三角形(壁の真横から見た線)にも使える)
	float DistancePointTriangle(float px, float pz, const Vector3 p[3])
	{
		// 内側判定(符号がそろっていれば内側)
		const auto side = [](float px, float pz, const Vector3& a, const Vector3& b)
		{
			return (b.x - a.x) * (pz - a.z) - (b.z - a.z) * (px - a.x);
		};
		const float s0 = side(px, pz, p[0], p[1]);
		const float s1 = side(px, pz, p[1], p[2]);
		const float s2 = side(px, pz, p[2], p[0]);
		const bool hasNeg = (s0 < 0.0f) || (s1 < 0.0f) || (s2 < 0.0f);
		const bool hasPos = (s0 > 0.0f) || (s1 > 0.0f) || (s2 > 0.0f);
		if (!(hasNeg && hasPos)) { return 0.0f; }

		const float d0 = DistancePointSegment(px, pz, p[0].x, p[0].z, p[1].x, p[1].z);
		const float d1 = DistancePointSegment(px, pz, p[1].x, p[1].z, p[2].x, p[2].z);
		const float d2 = DistancePointSegment(px, pz, p[2].x, p[2].z, p[0].x, p[0].z);
		return (std::min)(d0, (std::min)(d1, d2));
	}

	// これより水平に近い面(法線のYが大きい面)は、床・屋根・坂とみなして障害物にしない
	constexpr float WALL_NORMAL_Y_MAX = 0.7f;
}

void NavGrid::AddObstacleBox(const Vector3& center, const Vector3& halfSize, float rotYDeg)
{
	obstacles.push_back({ center, halfSize.Abs(), rotYDeg });
}

int NavGrid::LoadObstaclesCsv(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) { return -1; }

	int count = 0;
	std::string line;
	while (std::getline(file, line))
	{
		// コメントを除去し、カンマを空白にして読む
		const size_t comment = line.find('#');
		if (comment != std::string::npos) { line.erase(comment); }
		for (char& ch : line) { if (ch == ',') { ch = ' '; } }

		std::istringstream stream(line);
		float v[7] = {};
		int read = 0;
		while (read < 7 && (stream >> v[read])) { ++read; }

		// 回転(7個目)は省略可。6個未満の行は読み飛ばす
		if (read < 6) { continue; }

		AddObstacleBox(Vector3(v[0], v[1], v[2]), Vector3(v[3], v[4], v[5]), v[6]);
		++count;
	}

	return count;
}
void NavGrid::AddObstacleTriangle(const Vector3& a, const Vector3& b, const Vector3& c)
{
	triangles.push_back({ { a, b, c } });
}

void NavGrid::ClearObstacles(void)
{
	obstacles.clear();
	triangles.clear();
}

void NavGrid::Build(const Config& cfg)
{
	config = cfg;

	// 不正な設定の保護
	if (config.cellSize < 1.0f) { config.cellSize = 1.0f; }

	width = static_cast<int>(std::ceil((config.maxPos.x - config.minPos.x) / config.cellSize));
	height = static_cast<int>(std::ceil((config.maxPos.z - config.minPos.z) / config.cellSize));
	if (width <= 0 || height <= 0)
	{
		width = height = 0;
		walkable.clear();
		floorHeights.clear();
		opaque.clear();
		return;
	}

	// 全セル歩行可能・床は基準高さで初期化
	walkable.assign(static_cast<size_t>(width) * height, 1);
	floorHeights.assign(static_cast<size_t>(width) * height, config.floorY);
	opaque.assign(static_cast<size_t>(width) * height, 0);

	for (const ObstacleBox& box : obstacles)
	{
		Rasterize(box);
	}

	for (const ObstacleTriangle& tri : triangles)
	{
		RasterizeTriangle(tri);
	}

	// 建物の内部など、歩いて行けない場所を歩行不可にする
	if (config.keepOnlyReachable) { RestrictToReachable(config.reachableSeed); }
}

void NavGrid::Rasterize(const ObstacleBox& box)
{
	// 敵の体がぶつかる高さの範囲と重ならない障害物は無視する
	// (低い段差、頭上の張り出しなど)
	const float bottom = box.center.y - box.halfSize.y;
	const float top = box.center.y + box.halfSize.y;
	const float bodyMin = config.floorY + config.stepHeight;
	const float bodyMax = config.floorY + config.agentHeight;
	if (top <= bodyMin || bottom >= bodyMax) { return; }

	const float rad = box.rotYDeg * (DX_PI_F / 180.0f);
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	const float r = config.agentRadius;
	const float hx = box.halfSize.x;
	const float hz = box.halfSize.z;

	// 回転後のボックスのXZ範囲(膨らませた分を含む)
	const float extentX = std::abs(c) * hx + std::abs(s) * hz + r;
	const float extentZ = std::abs(s) * hx + std::abs(c) * hz + r;

	const float cs = config.cellSize;
	const int x0 = (std::max)(0, static_cast<int>(std::floor((box.center.x - extentX - config.minPos.x) / cs)));
	const int x1 = (std::min)(width - 1, static_cast<int>(std::floor((box.center.x + extentX - config.minPos.x) / cs)));
	const int y0 = (std::max)(0, static_cast<int>(std::floor((box.center.z - extentZ - config.minPos.z) / cs)));
	const int y1 = (std::min)(height - 1, static_cast<int>(std::floor((box.center.z + extentZ - config.minPos.z) / cs)));

	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			// セル中心をボックスのローカル座標(XZ)へ
			const float dx = config.minPos.x + (x + 0.5f) * cs - box.center.x;
			const float dz = config.minPos.z + (y + 0.5f) * cs - box.center.z;
			const float lx = dx * c - dz * s;
			const float lz = dx * s + dz * c;

			// ボックスまでの距離が半径以内なら通れない(角は丸く膨らむ)
			const float ox = (std::max)(std::abs(lx) - hx, 0.0f);
			const float oz = (std::max)(std::abs(lz) - hz, 0.0f);
			if (ox * ox + oz * oz <= r * r)
			{
				walkable[Index(x, y)] = 0;
			}
		}
	}
}

void NavGrid::RasterizeTriangle(const ObstacleTriangle& tri)
{
	const Vector3* p = tri.p;

	// 立っている面(壁)だけを対象にする
	const Vector3 normal = (p[1] - p[0]).Cross(p[2] - p[0]);
	const float normalLength = normal.Length();
	if (normalLength < 1.0e-6f) { return; }
	if (std::abs(normal.y) / normalLength >= WALL_NORMAL_Y_MAX) { return; }

	const float yMin = (std::min)(p[0].y, (std::min)(p[1].y, p[2].y));
	const float yMax = (std::max)(p[0].y, (std::max)(p[1].y, p[2].y));

	// 敵の体の高さと重なれば、通行を妨げる
	const bool blocksBody = (yMax > config.floorY + config.stepHeight) && (yMin < config.floorY + config.agentHeight);

	// 目の高さ(身長の半分?9割)と重なれば、視線を遮る
	const bool blocksSight = (yMax > config.floorY + config.agentHeight * 0.5f) && (yMin < config.floorY + config.agentHeight * 0.9f);

	if (!blocksBody && !blocksSight) { return; }

	// セルの中心で判定するので、薄い壁を取りこぼさないよう、半径はセルの大きさに合わせて大きめにとる
	const float cs = config.cellSize;
	const float bodyRadius = (std::max)(config.agentRadius, cs * 0.6f);
	const float sightRadius = cs * 0.5f;
	const float reach = (std::max)(bodyRadius, sightRadius);

	const float minX = (std::min)(p[0].x, (std::min)(p[1].x, p[2].x)) - reach;
	const float maxX = (std::max)(p[0].x, (std::max)(p[1].x, p[2].x)) + reach;
	const float minZ = (std::min)(p[0].z, (std::min)(p[1].z, p[2].z)) - reach;
	const float maxZ = (std::max)(p[0].z, (std::max)(p[1].z, p[2].z)) + reach;

	const int x0 = (std::max)(0, static_cast<int>(std::floor((minX - config.minPos.x) / cs)));
	const int x1 = (std::min)(width - 1, static_cast<int>(std::floor((maxX - config.minPos.x) / cs)));
	const int y0 = (std::max)(0, static_cast<int>(std::floor((minZ - config.minPos.z) / cs)));
	const int y1 = (std::min)(height - 1, static_cast<int>(std::floor((maxZ - config.minPos.z) / cs)));

	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			const float cx = config.minPos.x + (x + 0.5f) * cs;
			const float cz = config.minPos.z + (y + 0.5f) * cs;
			const float d = DistancePointTriangle(cx, cz, p);

			if (blocksBody && d <= bodyRadius) { walkable[Index(x, y)] = 0; }
			if (blocksSight && d <= sightRadius) { opaque[Index(x, y)] = 1; }
		}
	}
}

void NavGrid::RestrictToReachable(const Vector3& seed)
{
	// 出発点が壁の中なら、近くの歩けるセルから始める
	const std::optional<Vector2I> start = FindNearestWalkable(WorldToCell(seed), 20);
	if (!start) { return; }

	// 4方向に広げながら、行けるセルに印を付ける
	std::vector<uint8_t> reached(walkable.size(), 0);
	std::vector<int> stack;
	stack.push_back(Index(start->x, start->y));
	reached[stack.back()] = 1;

	while (!stack.empty())
	{
		const int current = stack.back();
		stack.pop_back();
		const int cx = current % width;
		const int cy = current / width;

		const int dx[4] = { 1, -1, 0, 0 };
		const int dy[4] = { 0, 0, 1, -1 };
		for (int d = 0; d < 4; ++d)
		{
			const int nx = cx + dx[d];
			const int ny = cy + dy[d];
			if (!IsWalkable(nx, ny)) { continue; }
			const int next = Index(nx, ny);
			if (reached[next]) { continue; }
			reached[next] = 1;
			stack.push_back(next);
		}
	}

	// 行けなかったセルは歩行不可にする
	for (size_t i = 0; i < walkable.size(); ++i)
	{
		if (!reached[i]) { walkable[i] = 0; }
	}
}

int NavGrid::CountWalkable(void) const
{
	int count = 0;
	for (uint8_t w : walkable) { count += (w != 0) ? 1 : 0; }
	return count;
}

bool NavGrid::InBounds(const Vector2I& cell) const
{
	return cell.x >= 0 && cell.y >= 0 && cell.x < width && cell.y < height;
}

bool NavGrid::IsWalkable(const Vector2I& cell) const
{
	return IsWalkable(cell.x, cell.y);
}

bool NavGrid::IsWalkable(int x, int y) const
{
	if (x < 0 || y < 0 || x >= width || y >= height) { return false; }
	return walkable[Index(x, y)] != 0;
}

Vector2I NavGrid::WorldToCell(const Vector3& worldPos) const
{
	return Vector2I(
		static_cast<int>(std::floor((worldPos.x - config.minPos.x) / config.cellSize)),
		static_cast<int>(std::floor((worldPos.z - config.minPos.z) / config.cellSize))
	);
}

Vector3 NavGrid::CellToWorld(const Vector2I& cell) const
{
	return Vector3(
		config.minPos.x + (cell.x + 0.5f) * config.cellSize,
		GetFloorY(cell),
		config.minPos.z + (cell.y + 0.5f) * config.cellSize
	);
}

float NavGrid::GetFloorY(const Vector2I& cell) const
{
	if (!InBounds(cell)) { return config.floorY; }
	return floorHeights[Index(cell.x, cell.y)];
}

std::optional<Vector2I> NavGrid::FindNearestWalkable(const Vector2I& cell, int maxRadius) const
{
	if (IsWalkable(cell)) { return cell; }

	std::optional<Vector2I> best;
	float bestDistSq = 0.0f;

	for (int ring = 1; ring <= maxRadius; ++ring)
	{
		// 見つかった候補より外側のリングはこれ以上近くならない
		if (best && static_cast<float>(ring) > std::sqrt(bestDistSq)) { break; }

		for (int dy = -ring; dy <= ring; ++dy)
		{
			for (int dx = -ring; dx <= ring; ++dx)
			{
				// リングの外周のみ調べる
				if ((std::max)(std::abs(dx), std::abs(dy)) != ring) { continue; }

				const Vector2I candidate(cell.x + dx, cell.y + dy);
				if (!IsWalkable(candidate)) { continue; }

				const float distSq = static_cast<float>(dx * dx + dy * dy);
				if (!best || distSq < bestDistSq)
				{
					best = candidate;
					bestDistSq = distSq;
				}
			}
		}
	}

	return best;
}

bool NavGrid::IsLineWalkable(const Vector3& from, const Vector3& to) const
{
	const float dx = to.x - from.x;
	const float dz = to.z - from.z;
	const float length = std::sqrt(dx * dx + dz * dz);

	// セルの半分の間隔でサンプリングして、通れないセルを飛び越さないようにする
	const float step = config.cellSize * 0.5f;
	const int count = (std::max)(1, static_cast<int>(std::ceil(length / step)));

	for (int i = 0; i <= count; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(count);
		const Vector3 p(from.x + dx * t, 0.0f, from.z + dz * t);
		if (!IsWalkable(WorldToCell(p))) { return false; }
	}

	return true;
}

bool NavGrid::HasLineOfSight(const Vector3& from, const Vector3& to) const
{
	const float segMinY = (std::min)(from.y, to.y);
	const float segMaxY = (std::max)(from.y, to.y);

	for (const ObstacleBox& box : obstacles)
	{
		// 視線の高さと重ならない障害物は遮らない
		if (segMaxY < box.center.y - box.halfSize.y || segMinY > box.center.y + box.halfSize.y) { continue; }

		// 線分をボックスのローカル座標(XZ)へ
		const float rad = box.rotYDeg * (DX_PI_F / 180.0f);
		const float c = std::cos(rad);
		const float s = std::sin(rad);

		const float ax = from.x - box.center.x;
		const float az = from.z - box.center.z;
		const float bx = to.x - box.center.x;
		const float bz = to.z - box.center.z;

		const float la[2] = { ax * c - az * s, ax * s + az * c };
		const float lb[2] = { bx * c - bz * s, bx * s + bz * c };
		const float half[2] = { box.halfSize.x, box.halfSize.z };

		// 線分と矩形の交差判定(スラブ法)
		float t0 = 0.0f;
		float t1 = 1.0f;
		bool hit = true;
		for (int axis = 0; axis < 2 && hit; ++axis)
		{
			const float d = lb[axis] - la[axis];
			if (std::abs(d) < 1.0e-6f)
			{
				// この軸方向には動かない: 矩形の外にあれば当たらない
				if (std::abs(la[axis]) > half[axis]) { hit = false; }
			}
			else
			{
				float ta = (-half[axis] - la[axis]) / d;
				float tb = (half[axis] - la[axis]) / d;
				if (ta > tb) { std::swap(ta, tb); }
				t0 = (std::max)(t0, ta);
				t1 = (std::min)(t1, tb);
				if (t0 > t1) { hit = false; }
			}
		}

		if (hit) { return false; }
	}

	// 三角形(ステージの壁)由来の遮蔽。両端の足元のセルは、壁の近くに立っていても見えるよう調べない
	if (!opaque.empty())
	{
		const Vector2I fromCell = WorldToCell(from);
		const Vector2I toCell = WorldToCell(to);

		const float dx = to.x - from.x;
		const float dz = to.z - from.z;
		const float length = std::sqrt(dx * dx + dz * dz);
		const int count = (std::max)(1, static_cast<int>(std::ceil(length / (config.cellSize * 0.5f))));

		for (int i = 0; i <= count; ++i)
		{
			const float t = static_cast<float>(i) / static_cast<float>(count);
			const Vector2I cell = WorldToCell(Vector3(from.x + dx * t, 0.0f, from.z + dz * t));
			if (cell == fromCell || cell == toCell) { continue; }
			if (!InBounds(cell)) { continue; }
			if (opaque[Index(cell.x, cell.y)] != 0) { return false; }
		}
	}

	return true;
}

void NavGrid::DrawDebug(bool showWalkable) const
{
	if (!IsBuilt()) { return; }

	const unsigned int blockedColor = GetColor(255, 60, 60);
	const unsigned int walkableColor = GetColor(60, 255, 60);
	const float cs = config.cellSize;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			const float h = floorHeights[Index(x, y)] + 2.0f;
			const float x0 = config.minPos.x + x * cs;
			const float z0 = config.minPos.z + y * cs;

			if (walkable[Index(x, y)] == 0)
			{
				// 通れないセルは枠と対角線で描く
				const VECTOR a = VGet(x0, h, z0);
				const VECTOR b = VGet(x0 + cs, h, z0);
				const VECTOR c = VGet(x0 + cs, h, z0 + cs);
				const VECTOR d = VGet(x0, h, z0 + cs);
				DrawLine3D(a, b, blockedColor);
				DrawLine3D(b, c, blockedColor);
				DrawLine3D(c, d, blockedColor);
				DrawLine3D(d, a, blockedColor);
				DrawLine3D(a, c, blockedColor);
				DrawLine3D(b, d, blockedColor);
			}
			else if (showWalkable)
			{
				// 歩けるセルは中心に短い縦線
				const float cx = x0 + cs * 0.5f;
				const float cz = z0 + cs * 0.5f;
				DrawLine3D(VGet(cx, h, cz), VGet(cx, h + cs * 0.15f, cz), walkableColor);
			}
		}
	}
}
