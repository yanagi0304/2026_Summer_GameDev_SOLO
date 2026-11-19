#include "EnemyManager.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "../../pch.h"

#include "../../Manager/TimeScale/TimeScale.h"

#include "Navigation/NavGrid.h"
#include "Navigation/NavManager.h"

namespace
{
	// 単位はcm

	// 最初のウェーブが始まるまで / ウェーブ間の休憩(フレーム)
	constexpr float START_DELAY_FRAMES = 120.0f;
	constexpr float REST_FRAMES = 180.0f;

	// ウェーブの内容(敵の数, 1体ずつ出す間隔[フレーム])
	const std::vector<WaveController::Wave> WAVES =
	{
		{ 2, 60.0f },
		{ 4, 45.0f },
		{ 6, 30.0f },
	};

	// 出現位置の、目標からの距離の範囲
	constexpr float SPAWN_MIN_DIST = 2000.0f;
	constexpr float SPAWN_MAX_DIST = 4500.0f;

	// 他の敵とこれ以上近い場所には出さない
	constexpr float SPAWN_MIN_GAP = 150.0f;

	// 出現位置の探索回数 / そのうち「目標から見えない場所」だけを探す回数
	constexpr int SPAWN_TRY_COUNT = 40;
	constexpr int SPAWN_HIDDEN_ONLY_TRY_COUNT = 25;

	// 目の高さ(視線の遮蔽判定に使う)
	constexpr float EYE_HEIGHT = 160.0f;
	constexpr float TARGET_AIM_HEIGHT = 120.0f;

	// 目標よりこれだけ低い所まで落ちた敵は、倒された扱いにする(ウェーブが止まらないように)
	constexpr float FALL_LIMIT = 5000.0f;
}

EnemyManager::EnemyManager(const NavGrid& grid, NavManager& nav, const Vector3* targetPos, ActorFunc addActor, ActorFunc removeActor,
	std::function<void(const Vector3&)> onHitPlayer) :
	grid(grid),
	nav(nav),
	targetPos(targetPos),
	addActor(addActor),
	removeActor(removeActor),
	baseModel(MV1LoadModel("Data/Model/Character/WoChamolenium/WoChamolenium.mv1")),
	swordModel(MV1LoadModel("Data/Model/Weapon/Kogetsu.mv1")),
	onHitPlayer(onHitPlayer),

	wave(WAVES, START_DELAY_FRAMES, REST_FRAMES),
	enemies(),
	pendingSpawn(0),
	rng(std::random_device{}()),
	lastSpawnMs(0.0)
{
}

EnemyManager::~EnemyManager(void)
{
	// 複製した敵のモデルは、シーンがActorを解放する時に消える(複製元は最後まで残す)
	if (baseModel >= 0) { MV1DeleteModel(baseModel); }
	if (swordModel >= 0) { MV1DeleteModel(swordModel); }
}

void EnemyManager::Update(void)
{
	if (targetPos == nullptr || !grid.IsBuilt()) { return; }

	// 倒された敵、ステージの外へ落ちた敵を取り除く
	for (size_t i = 0; i < enemies.size();)
	{
		Enemy* enemy = enemies[i];

		if (!enemy->IsDefeated() && enemy->GetPos().y < targetPos->y - FALL_LIMIT)
		{
			enemy->Defeat();
		}

		if (enemy->IsDefeated())
		{
			// 先に一覧から外してから解放する
			enemies.erase(enemies.begin() + i);
			removeActor(enemy);
		}
		else
		{
			++i;
		}
	}

	// ウェーブを進めて、出現させる数を受け取る(出現待ちも「生きている」数に含める)
	pendingSpawn += wave.Update(static_cast<int>(enemies.size()) + pendingSpawn, TimeScale::Get());

	// 出現させる。位置が見つからなければ次のフレームにまた試す
	while (pendingSpawn > 0)
	{
		Vector3 pos;
		if (!PickSpawnPos(pos)) { break; }
		Spawn(pos);
		--pendingSpawn;
	}
}

void EnemyManager::DefeatAll(void)
{
	for (Enemy* enemy : enemies) { enemy->Defeat(); }
}

int EnemyManager::GetAliveCount(void)const
{
	int count = 0;
	for (const Enemy* enemy : enemies)
	{
		if (!enemy->IsDefeated()) { ++count; }
	}
	return count;
}

void EnemyManager::DrawDebug(void)const
{
	for (const Enemy* enemy : enemies) { enemy->DrawDebug(); }
}

bool EnemyManager::PickSpawnPos(Vector3& out)
{
	std::uniform_int_distribution<int> distX(0, grid.GetWidth() - 1);
	std::uniform_int_distribution<int> distY(0, grid.GetHeight() - 1);

	const Vector3 aim = *targetPos + Vector3(0.0f, TARGET_AIM_HEIGHT, 0.0f);

	for (int attempt = 0; attempt < SPAWN_TRY_COUNT; ++attempt)
	{
		const Vector2I cell(distX(rng), distY(rng));
		if (!grid.IsWalkable(cell)) { continue; }

		// 高さはそのセルの床に合わせる(少し浮かせて、重力で床に落とす)
		Vector3 pos = grid.CellToWorld(cell);
		pos.y += 20.0f;

		// 目標からの距離
		const float dx = pos.x - targetPos->x;
		const float dz = pos.z - targetPos->z;
		const float dist = std::sqrt(dx * dx + dz * dz);
		if (dist < SPAWN_MIN_DIST || dist > SPAWN_MAX_DIST) { continue; }

		// 最初のうちは、目標から見えない場所だけを選ぶ(目の前に湧かないように)
		if (attempt < SPAWN_HIDDEN_ONLY_TRY_COUNT &&
			grid.HasLineOfSight(pos + Vector3(0.0f, EYE_HEIGHT, 0.0f), aim))
		{
			continue;
		}

		// 他の敵と重ならない場所
		bool tooClose = false;
		for (const Enemy* other : enemies)
		{
			const float ox = pos.x - other->GetPos().x;
			const float oz = pos.z - other->GetPos().z;
			if (ox * ox + oz * oz < SPAWN_MIN_GAP * SPAWN_MIN_GAP) { tooClose = true; break; }
		}
		if (tooClose) { continue; }

		out = pos;
		return true;
	}

	return false;
}

void EnemyManager::Spawn(const Vector3& pos)
{
	const auto start = std::chrono::high_resolution_clock::now();

	Enemy::Combat combat;
	combat.swordModel = swordModel;
	combat.onHitPlayer = onHitPlayer;

	Enemy* enemy = new Enemy(Enemy::NavContext{ &grid, &nav }, targetPos, pos, &enemies, baseModel, combat);
	enemies.push_back(enemy);

	// シーンに追加する(読み込み・初期化・当たり判定の登録はシーン側で行われる)
	addActor(enemy);

	lastSpawnMs = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count();
}
