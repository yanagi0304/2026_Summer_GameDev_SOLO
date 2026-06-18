#pragma once
#include <vector>
#include "../../../Common/Vector3.h"
#include "../../../Manager/Camera/Camera.h"
#include "../../Mannequin/Mannequin.h"


class LockOn
{
public:

	static constexpr float LOCK_ON_DISTANCE = 500.0f; // ロックオンの有効距離

public:

	LockOn();
	~LockOn() = default;

	void Load(void);
	//プレイヤーの座標とマネキンの座標を受け取り、ロックオン対象を更新する
	void Update(const Vector3& PlayerPos,Mannequin* mannequins);
	//ターゲットの頭上にマークを表示
	void Draw(void);

private:

	Mannequin* targetMannequin = nullptr; // ロックオン対象のマネキン

	// ロックオン対象のマネキンの頭上に表示するマークの画像
	int targetImgCandidate_;

};

