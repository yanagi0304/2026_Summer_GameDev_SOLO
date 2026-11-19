#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../../../Common/Vector2I.h"
#include "../../../Common/Vector3.h"

// 経路探索用の2Dグリッド(XZ平面)
// ステージの形状(メッシュ/ボックス)には依存せず、障害物ボックスのリストから生成する
// 各セルは「歩行可否」と「床の高さ」を持つ(屋上・段差対応の拡張用)
class NavGrid
{
public:

	// グリッド生成設定(単位はワールド座標と同じ)
	struct Config
	{
		// 対象範囲(XZ)
		Vector3 minPos = Vector3(-5000.0f, 0.0f, -5000.0f);
		Vector3 maxPos = Vector3(5000.0f, 0.0f, 5000.0f);

		// セルの一辺
		float cellSize = 200.0f;

		// 床の基準高さ
		float floorY = 0.0f;

		// 敵の半径(障害物をこの分だけ膨らませる)
		float agentRadius = 50.0f;

		// 敵の身長(これより高い位置にある障害物は無視する)
		float agentHeight = 180.0f;

		// 乗り越えられる段差(これより低い障害物は無視する)
		float stepHeight = 30.0f;

		// trueなら、reachableSeedから歩いて行けないセル(建物の内部、囲まれた場所)を歩行不可にする
		bool keepOnlyReachable = false;
		Vector3 reachableSeed = Vector3(0.0f, 0.0f, 0.0f);
	};

	NavGrid(void) = default;
	~NavGrid(void) = default;

	// 障害物ボックスを登録(Build前に呼ぶ)
	// center: 中心 / halfSize: 各軸の半分の大きさ / rotYDeg: Y軸回転(度。Unityのeulers.yと同じ向き)
	void AddObstacleBox(const Vector3& center, const Vector3& halfSize, float rotYDeg = 0.0f);

	// 三角形を障害物として登録する(ステージのメッシュなど。Build前に呼ぶ)
	// 壁のように立っていて、敵の体の高さと重なる三角形だけが障害物になる(床や屋根は無視される)
	void AddObstacleTriangle(const Vector3& a, const Vector3& b, const Vector3& c);
	// 障害物ボックスをCSVファイルから読み込んで登録する(Build前に呼ぶ)
	// 1行1ボックス: centerX,centerY,centerZ,halfX,halfY,halfZ,rotYDeg
	// 空行と、#で始まる行は無視する。読めた行数を返し、ファイルが開けなければ-1を返す
	int LoadObstaclesCsv(const std::string& path);
	// 登録済みの障害物を消す
	void ClearObstacles(void);

	// 登録された障害物からグリッドを構築
	void Build(const Config& config);

	// ---- 問い合わせ ----

	bool IsBuilt(void) const { return !walkable.empty(); }

	// 歩行可能なセルの数(デバッグ表示用)
	int CountWalkable(void) const;
	int GetWidth(void) const { return width; }
	int GetHeight(void) const { return height; }
	float GetCellSize(void) const { return config.cellSize; }
	const Config& GetConfig(void) const { return config; }

	// セル範囲内か
	bool InBounds(const Vector2I& cell) const;

	// 歩けるセルか(範囲外はfalse)
	bool IsWalkable(const Vector2I& cell) const;
	bool IsWalkable(int x, int y) const;

	// ワールド座標 → セル(範囲外でもそのまま計算する。InBoundsで確認すること)
	Vector2I WorldToCell(const Vector3& worldPos) const;

	// セル中心のワールド座標(Yはセルの床の高さ)
	Vector3 CellToWorld(const Vector2I& cell) const;

	// セルの床の高さ
	float GetFloorY(const Vector2I& cell) const;

	// 指定セルから最も近い歩行可能セルを探す(maxRadius: セル数での探索半径)
	std::optional<Vector2I> FindNearestWalkable(const Vector2I& cell, int maxRadius) const;

	// 2点間を直線で歩けるか(間のセルがすべて歩行可能か)
	bool IsLineWalkable(const Vector3& from, const Vector3& to) const;

	// 2点間の視線が障害物(ビルなど)に遮られていないか。半径による膨らませは行わない
	// 高さも見るので、低い段差の上は通る(視線の高さと重なる障害物だけが遮る)
	bool HasLineOfSight(const Vector3& from, const Vector3& to) const;
	// デバッグ描画(通れないセルを赤で描く)
	void DrawDebug(bool showWalkable = false) const;

private:

	// 障害物ボックス
	struct ObstacleBox
	{
		Vector3 center;
		Vector3 halfSize;
		float rotYDeg;
	};

	// 三角形(障害物の候補)
	struct ObstacleTriangle
	{
		Vector3 p[3];
	};

	// 1つのボックスを歩行可否に反映
	void Rasterize(const ObstacleBox& box);

	// 1つの三角形を歩行可否と視線の遮蔽に反映
	void RasterizeTriangle(const ObstacleTriangle& tri);

	// 指定位置から歩いて行けないセルを、歩行不可にする
	void RestrictToReachable(const Vector3& seed);

	int Index(int x, int y) const { return y * width + x; }

	Config config;
	int width = 0;
	int height = 0;

	std::vector<ObstacleBox> obstacles;
	std::vector<ObstacleTriangle> triangles;

	// 1=歩ける 0=通れない
	std::vector<uint8_t> walkable;

	// セルごとの床の高さ
	std::vector<float> floorHeights;

	// 1=視線を遮る壁がある(三角形から生成。敵が壁越しに見えないようにするため)
	std::vector<uint8_t> opaque;
};
