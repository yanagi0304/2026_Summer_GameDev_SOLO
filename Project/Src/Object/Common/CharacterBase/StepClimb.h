#pragma once

#include <functional>
#include <vector>

#include "../../../Common/Vector3.h"

// 段差・斜面の自動登り(物理や衝突の仕組みには依存しない純粋な計算)
//
// 考え方: 形状(縁石・面取り・坂・庇)を解析せず、
//   「押しているのに、前へ進めていない間は、少しずつ持ち上げる」だけにする。
//   縁石でも、面取りされた縁でも、坂でも、同じように登れる。
//   持ち上げすぎ(壁登り)を防ぐ最低限の条件だけを持つ。
//
// 単位はcm。プレイヤーも敵も、同じ判定を使う

// 判定の設定
struct StepClimbParams
{
	// 登れる高さの上限(これを超える壁は登らない)。1回の押しつけで持ち上げる合計の上限でもある
	float maxHeight = 50.0f;

	// キャラクターのコライダーの半径
	float radius = 40.0f;

	// 持ち上げを始める時に、足元から地面までの距離がこれ以内であること(空中では始めない)
	float maxGroundClearance = 20.0f;

	// 1フレームに持ち上げる高さ(大きいほど速く登るが、跳ねて見える)
	float liftPerFrame = 6.0f;

	// この回数続けて「進めていない」と、持ち上げを始める(加速中の一瞬を、押しつけと間違えないため)
	int blockedFramesRequired = 3;

	// 入力した向きに進めた距離が、目標の移動量のこの割合未満なら「進めていない」とみなす
	float progressRatio = 0.7f;

	// 前方の障害物を調べるレイの、地面からの高さ。低い方から順に飛ばして、最初に当たった高さを障害物とする
	// 縁石の板の下が空いていて、面が地面から浮いている形状でも見つけられるよう、複数の高さを使う
	int probeCount = 5;
	float probeHeights[5] = { 2.0f, 8.0f, 16.0f, 28.0f, 45.0f };
};

// レイの結果
struct StepRayHit
{
	bool hit = false;
	float distance = 0.0f;
	Vector3 point = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
};

// 判定の結果(なぜ持ち上げた/持ち上げなかったか。デバッグ表示用)
enum class StepClimbResult
{
	Disabled,          // 段差を登る設定が無効
	NotPressing,       // 移動の入力をしていない
	Progressing,       // 入力した向きに進めている(登る必要がない)
	NotBlockedYet,     // 進めていないが、まだ数フレームしか続いていない
	NotGrounded,       // 空中にいる(持ち上げは、地面から始める)
	NoStageObstacle,   // 前方にステージの障害物がない(敵や他のキャラに止められている)
	WallTooTall,       // 登れる高さを超える壁
	LimitReached,      // 1回の押しつけで持ち上げられる上限に達した
	Lifting,           // 持ち上げた
};

// 判定のデバッグ情報
struct StepClimbDebug
{
	// 判定で飛ばしたレイ(3D描画用。直近の判定の分)
	struct Probe
	{
		Vector3 from = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 to = Vector3(0.0f, 0.0f, 0.0f);   // 当たった点。当たらなければレイの終点
		bool hit = false;
		int kind = 0;   // 0=直下の地面 1=前方の障害物 2=背の高い壁の判定
	};
	std::vector<Probe> probes;

	StepClimbResult result = StepClimbResult::Disabled;

	// 足元から直下の地面までの距離(地面に当たらなければ-1)
	float groundClearance = -1.0f;

	// 障害物までの距離 / この押しつけで持ち上げた合計
	float wallDistance = 0.0f;
	float lifted = 0.0f;

	// 入力した向きに進めた距離(直近のフレーム)
	float progress = 0.0f;
};

// 判定のフレーム間の状態(キャラクターごとに1つ持つ)
struct StepClimbState
{
	// この押しつけで、持ち上げた合計
	float lifted = 0.0f;

	// 続けて進めていないフレーム数
	int blockedFrames = 0;

	void Reset(void) { lifted = 0.0f; blockedFrames = 0; }
};

// レイを飛ばす関数(ステージの床・壁だけが対象になるようにして渡すこと)
using StepRayFunc = std::function<StepRayHit(const Vector3& origin, const Vector3& direction, float maxDistance)>;

/// <summary>
/// 毎フレーム呼び、このフレームに持ち上げる高さを返す
/// </summary>
/// <param name="pos">キャラクターの足元の座標(Transformのpos)</param>
/// <param name="inputDirection">入力した移動の向き(水平。長さは問わない)</param>
/// <param name="intendedSpeed">目標の移動速度(1フレームあたり。入力がなければ0)</param>
/// <param name="progress">入力した向きに、直近のフレームで実際に進めた距離</param>
/// <param name="params">設定</param>
/// <param name="ray">レイを飛ばす関数</param>
/// <param name="state">フレーム間の状態(呼び出しごとに更新される)</param>
/// <param name="debug">nullでなければ、判定の結果と理由を書き込む</param>
/// <returns>持ち上げる高さ。持ち上げないなら0</returns>
float ComputeForcedLift(const Vector3& pos, const Vector3& inputDirection, float intendedSpeed, float progress,
	const StepClimbParams& params, const StepRayFunc& ray, StepClimbState& state, StepClimbDebug* debug = nullptr);
