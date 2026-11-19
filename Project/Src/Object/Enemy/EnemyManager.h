#pragma once

#include <functional>
#include <random>
#include <vector>

#include "Enemy.h"
#include "WaveController.h"

class NavGrid;
class NavManager;
class ActorBase;

// 敵の出現・撤去とウェーブの進行を担当する
// シーンのActor管理(AddActor/RemoveActor)は、シーンから関数として渡してもらう
// (出現中にも当たり判定へ登録される必要があるため、シーンを経由する)
class EnemyManager
{
public:

	// シーンのActor追加・削除を呼ぶ関数
	using ActorFunc = std::function<void(ActorBase*)>;

	/// <param name="grid">経路探索グリッド(出現位置の選定に使う)</param>
	/// <param name="nav">経路探索の窓口</param>
	/// <param name="targetPos">敵が追う目標の座標(参照先はこのクラスより長く生きていること)</param>
	/// <param name="addActor">シーンにActorを追加する関数</param>
	/// <param name="removeActor">シーンからActorを外して解放する関数(Update中に呼べない点に注意)</param>
	/// <param name="onHitPlayer">敵の攻撃がプレイヤーに当たった時に呼ぶ関数(引数は、攻撃した敵の位置)</param>
	EnemyManager(const NavGrid& grid, NavManager& nav, const Vector3* targetPos, ActorFunc addActor, ActorFunc removeActor,
		std::function<void(const Vector3&)> onHitPlayer = nullptr);

	~EnemyManager(void);

	// 毎フレーム、シーンの更新の最後(Actor更新の後)に呼ぶ
	void Update(void);

	// 生きている敵をすべて倒した状態にする(デバッグ用。次のUpdateで撤去される)
	void DefeatAll(void);

	// 生きている敵の数
	int GetAliveCount(void)const;

	const WaveController& GetWave(void)const { return wave; }

	// 直近の出現(生成?シーンへの追加)にかかった時間(ミリ秒。計測用)
	double GetLastSpawnMs(void)const { return lastSpawnMs; }

	// 全敵のデバッグ描画
	void DrawDebug(void)const;

private:

	// 出現位置を選ぶ。見つからなければfalse
	bool PickSpawnPos(Vector3& out);

	// 敵を1体出現させる
	void Spawn(const Vector3& pos);

	const NavGrid& grid;
	NavManager& nav;
	const Vector3* targetPos;
	ActorFunc addActor;
	ActorFunc removeActor;

	// 敵のモデルの複製元(シーンの読み込み時に1回だけ読み込む)
	int baseModel;

	// 敵が持つ弧月のモデルの複製元(同じく、シーンの読み込み時に1回だけ読み込む)
	int swordModel;

	// 攻撃がプレイヤーに当たった時の通知
	std::function<void(const Vector3&)> onHitPlayer;

	// ウェーブの進行
	WaveController wave;

	// 出現中の敵(Actorの所有権はシーンにある)
	std::vector<Enemy*> enemies;

	// 出現させたいのに、位置が見つからず出せていない数
	int pendingSpawn;

	std::mt19937 rng;

	double lastSpawnMs;
};
