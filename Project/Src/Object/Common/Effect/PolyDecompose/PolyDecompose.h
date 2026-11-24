#pragma once

#include <vector>

#include "../../../../pch.h"

// ポリゴン分解エフェクト(ジオメトリシェーダ)
// アニメーションするMV1モデルを、三角形ごとにバラバラにして描く。
//   progress = 0 : 元のモデルどおり
//   progress = 1 : すべての破片が消えた状態
// 生成演出なら progress を 1→0 に、撃破演出なら 0→1 に動かす。
//
// 流れ: Init(model) → 毎フレーム Update() でスキニング → Draw(progress) → Release()
// スキニングはDXライブラリのCPUスキニング(MV1RefreshReferenceMesh)を使うので、
// 使うのは演出中のモデルだけにする(全員に常時かけると重い)。
class PolyDecompose
{
public:

	// 見た目の調整値
	struct Params
	{
		// 破片の移動量の倍率
		float spread = 1.0f;

		// 破片の回転量
		float spin = 6.0f;

		// 法線 0:スムース 1:フラット(0なら演出が終わった時の見た目とつながる)
		float flatMix = 0.0f;

		// 分解の前線 0:上→下 1:下→上 2:中心→外 3:ランダム
		// progressを1→0に動かす時は、前線の逆順で組み上がる(0なら足元から組み上がる)
		int sweepMode = 0;
	};

	PolyDecompose(void);
	~PolyDecompose(void) = default;

	// 初期化。modelはMV1のモデルハンドル(ワールド行列を設定済みであること)
	bool Init(int model);

	// スキニング済みの頂点で頂点バッファを更新する(モデルのワールド行列とアニメーションを反映してから呼ぶ)
	void Update(void);

	// 描画する。カメラはあらかじめ設定しておくこと
	void Draw(float progress, const Params& params);

	// 解放
	void Release(void);

	// 使える状態か
	bool IsValid(void)const { return vertexBuffer >= 0; }

private:

	// ジオメトリシェーダー・ピクセルシェーダー用の定数バッファ(PolyDecompose.hlsliのPolyDecomposeCBと同じ並び)
	struct alignas(16) ConstantData
	{
		float viewProj[16];

		float camPos[4];

		float time;
		float progress;
		float flatMix;
		float spread;

		float spin;
		float boundsMinY;
		float boundsMaxY;
		float rMax;

		float boundsCenter[4];

		float worldScale;
		int sweepMode;
		int useTexture;
		int pad0;
	};

	// マテリアルごとの描画単位
	struct Batch
	{
		int indexBuffer = -1;
		int texture = -1;
		int triangleCount = 0;
	};

	// 破片の移動量の基準にしたメッシュの大きさ(中心からの最大距離)
	static constexpr float REFERENCE_RADIUS = 2.1f;

	// 描画に使う定数バッファ(全インスタンスで共有)
	static int sharedConstantBuffer;
	static int sharedRefCount;

	// 元のモデル
	int model;

	// 頂点バッファ(スキニング結果を毎フレーム書き込む)
	int vertexBuffer;

	// 頂点の元データ(posとnormだけ毎フレーム更新する。sposは最初の姿勢で固定)
	std::vector<VERTEX3DSHADER> vertices;

	// マテリアルごとの描画単位
	std::vector<Batch> batches;

	// 共有の定数バッファを使う登録をしたか
	bool sharedAcquired;

	// 最初の姿勢を取り込んだか
	bool restCaptured;

	// 最初の姿勢でのバウンディング
	float boundsMinY;
	float boundsMaxY;
	float boundsRMax;
	VECTOR boundsCenter;

	// 経過時間(秒)。破片のちらつきに使う
	float time;

	// 定数バッファに渡す値
	ConstantData data;

	// 参照用メッシュから頂点を取り込む(restがtrueならsposも書き換える)
	void CaptureVertices(bool rest);

	// 最初の姿勢でのバウンディングを求める
	void ComputeBounds(void);
};
