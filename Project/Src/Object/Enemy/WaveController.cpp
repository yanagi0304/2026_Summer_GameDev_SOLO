#include "WaveController.h"

#include <algorithm>

WaveController::WaveController(const std::vector<Wave>& waves, float startDelayFrames, float restFrames) :
	waves(waves),
	restFrames(restFrames),

	phase(waves.empty() ? Phase::Finished : Phase::Resting),
	waveIndex(-1),
	remainingToSpawn(0),
	spawnTimer(0.0f),
	restTimer(startDelayFrames)
{
}

int WaveController::Update(int aliveCount, float frames)
{
	int spawnCount = 0;

	switch (phase)
	{
	case Phase::Resting:
		restTimer -= frames;
		if (restTimer <= 0.0f)
		{
			// 次のウェーブを始める
			++waveIndex;
			if (waveIndex >= static_cast<int>(waves.size()))
			{
				phase = Phase::Finished;
				break;
			}
			remainingToSpawn = waves[waveIndex].enemyCount;
			spawnTimer = 0.0f;
			phase = Phase::Spawning;
		}
		break;

	case Phase::Spawning:
	{
		const float interval = (std::max)(waves[waveIndex].spawnIntervalFrames, 1.0f);
		spawnTimer -= frames;
		while (remainingToSpawn > 0 && spawnTimer <= 0.0f)
		{
			++spawnCount;
			--remainingToSpawn;
			spawnTimer += interval;
		}

		// 出し終わったら、全員倒されるのを待つ(クリア判定は次のフレームから)
		if (remainingToSpawn <= 0) { phase = Phase::Fighting; }
		break;
	}

	case Phase::Fighting:
		if (aliveCount <= 0)
		{
			if (waveIndex + 1 >= static_cast<int>(waves.size()))
			{
				phase = Phase::Finished;
			}
			else
			{
				restTimer = restFrames;
				phase = Phase::Resting;
			}
		}
		break;

	case Phase::Finished:
		break;
	}

	return spawnCount;
}
