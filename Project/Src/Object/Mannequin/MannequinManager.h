#pragma once
#include "../Common/ActorBase/ActorBase.h"

class MannequinManager :
    public ActorBase
{
public:
	MannequinManager(const Vector3& targetPos);

	~MannequinManager()override = default;

	void Load(void)override;

private:

#pragma region Žó‚¯Žæ‚è
	const Vector3& targetPos;
#pragma endregion
};

