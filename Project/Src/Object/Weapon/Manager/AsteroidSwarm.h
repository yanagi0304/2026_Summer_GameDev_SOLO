#pragma once

#include <vector>

#include "../../../Common/Vector3.h"

class Asteroid;
struct Transform;

class AsteroidSwarm
{
public:

	// 1辺あたりの個数
	static constexpr int SIDE_NUM = 3;

	// 弾の総数
	static constexpr int BULLET_NUM = SIDE_NUM * SIDE_NUM * SIDE_NUM;

	// コンストラクタ
	AsteroidSwarm(void);

	~AsteroidSwarm(void) = default;

	// 弾を1発登録する
	void Add(Asteroid* bullet);

	// すべて消す
	void HideAll(void);

	// 1個だけ手元に出す
	// center: 出す位置
	void ShowOne(const Vector3& center);

	// 出している1個を、手元の位置へ追従させる
	void FollowOne(const Vector3& center);

	// 全部を出す
	void ShowFormation(const Transform& ownerTrans, const Vector3& center);

	// 全部を立方体の形に並べる
	// ownerTrans: 持ち主の向き / center: 立方体の中心
	void FollowFormation(const Transform& ownerTrans, const Vector3& center);

	// すべて撃つ
	void FireAll(const Vector3& dir);

	// すべて手元の状態に戻す
	void RecallAll(void);

private:

	// 弾どうしの間隔
	static constexpr float SPACE = 25.0f;

	// 並べたときの弾の大きさ
	static constexpr float SPLIT_SCALE = 0.3f;

	// 登録した弾
	std::vector<Asteroid*> bullets_;

	// 立方体の中での各弾の位置(中心を原点にした格子座標。-1, 0, +1)
	std::vector<Vector3> offsets_;

};
