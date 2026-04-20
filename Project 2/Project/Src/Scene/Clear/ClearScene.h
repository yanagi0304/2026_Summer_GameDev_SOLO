#pragma once

#include"../SceneBase.h"

class SkyDome;

class ClearScene : public SceneBase
{
public:
	ClearScene();
	~ClearScene()override = default;

	void Load(void)override;
	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;
	void Release(void)override;

private:
};
