#pragma once

#include <vector>

// ウェーブの進行管理(出現数と休憩の時間だけを扱い、敵そのものは知らない)
//
// 流れ: 開始待ち → [出現 → 戦闘(全員倒すまで) → 休憩] をウェーブ数だけ繰り返す → 終了
// 毎フレームUpdateを呼び、戻り値の数だけ敵を出現させる
class WaveController
{
public:

	// 1ウェーブの設定
	struct Wave
	{
		// 出現させる敵の数
		int enemyCount;

		// 敵を1体ずつ出現させる間隔(フレーム)
		float spawnIntervalFrames;
	};

	enum class Phase
	{
		Resting,	// 開始待ち / ウェーブ間の休憩
		Spawning,	// 出現中
		Fighting,	// 出現し終わって、全員倒されるのを待っている
		Finished,	// 全ウェーブ終了
	};

	/// <param name="waves">ウェーブの一覧(順番に進む)</param>
	/// <param name="startDelayFrames">最初のウェーブが始まるまでのフレーム数</param>
	/// <param name="restFrames">ウェーブをクリアしてから次が始まるまでのフレーム数</param>
	WaveController(const std::vector<Wave>& waves, float startDelayFrames, float restFrames);

	/// <summary>
	/// 毎フレーム呼ぶ
	/// </summary>
	/// <param name="aliveCount">いま生きている敵の数(出現待ちの数を含める)</param>
	/// <param name="frames">経過フレーム数(通常は1。時間倍率を掛けた値)</param>
	/// <returns>このフレームに出現させる敵の数</returns>
	int Update(int aliveCount, float frames);

	Phase GetPhase(void)const { return phase; }
	bool IsFinished(void)const { return phase == Phase::Finished; }

	// 現在のウェーブ番号(1始まり。開始前は0) / ウェーブの総数
	int GetWaveNumber(void)const { return waveIndex + 1; }
	int GetWaveTotal(void)const { return static_cast<int>(waves.size()); }

	// このウェーブで、まだ出現させていない敵の数
	int GetRemainingToSpawn(void)const { return remainingToSpawn; }

	// 休憩の残りフレーム
	float GetRestRemainingFrames(void)const { return restTimer; }

private:

	std::vector<Wave> waves;
	float restFrames;

	Phase phase;
	int waveIndex;
	int remainingToSpawn;
	float spawnTimer;
	float restTimer;
};
