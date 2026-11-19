#include "NavGrid.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

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
void NavGrid::ClearObstacles(void)
{
	obstacles.clear();
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
		return;
	}

	// 全セル歩行可能・床は基準高さで初期化
	walkable.assign(static_cast<size_t>(width) * height, 1);
	floorHeights.assign(static_cast<size_t>(width) * height, config.floorY);

	for (const ObstacleBox& box : obstacles)
	{
		Rasterize(box);
	}
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
