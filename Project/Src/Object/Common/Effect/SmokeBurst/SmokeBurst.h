#pragma once

#include <array>

#include "../../../../pch.h"
#include "../../../../Common/Vector3.h"

#include "../../ActorBase/ActorBase.h"

// 煙がぶわっと弾ける演出のアクター
// 板ポリを描くだけで、動きはSmokeBurstShader(頂点シェーダー)が計算する
// 使い方: 親アクターが AddChildActor で持ち、Emit(位置, 向き) を呼ぶ
class SmokeBurst : public ActorBase
{
public:

	SmokeBurst();
	~SmokeBurst() override = default;

	// 読み込み処理(粒の頂点データを作る)
	void Load(void) override;

	// 煙を発生させる
	// pos: 発生位置 / normal: 煙が広がる側の向き(盾の正面など)
	void Emit(const Vector3& pos, const Vector3& normal);

private:

	// 1回の煙を作る粒の数
	static constexpr int PUFF_NUM = 20;

	// 粒1つあたりの頂点数(板ポリ)
	static constexpr int VERTEX_PER_PUFF = 4;

	// 頂点の総数
	static constexpr int VERTEX_NUM = PUFF_NUM * VERTEX_PER_PUFF;

	// インデックスの総数(板ポリ1枚あたり三角形2つ)
	static constexpr int INDEX_NUM = PUFF_NUM * 6;

	// 三角形の総数
	static constexpr int POLYGON_NUM = PUFF_NUM * 2;

	// 更新(煙が消える時間になったら描画を止める)
	void SubUpdate(void) override;

	// 描画(板ポリを描く。シェーダーはActorBaseが適用済み)
	void SubDraw(void) override;

	// 半透明のアクターとして描画する
	ACTOR_DRAW_TYPE DefaultDrawType(void) const override { return ACTOR_DRAW_TYPE::Alpha; }

	// 粒の頂点データを作る
	void CreateVertices(void);

	// 粒の頂点データ(板ポリが PUFF_NUM 枚)
	std::array<VERTEX3DSHADER, VERTEX_NUM> vertices;

	// 粒のインデックス
	std::array<unsigned short, INDEX_NUM> indices;

	// 発生してからの経過時間(秒)
	float elapsed;

	// 発生させた回数(乱数のばらつきに使う)
	int emitCount;
};
