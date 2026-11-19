#pragma once

#include <functional>
#include <vector>

#include "../../../Common/Vector3.h"

// 段差を登る判定(物理や衝突の仕組みには依存しない純粋な計算)
// 足元から進行方向にレイを飛ばして、低い壁の上面が登れる高さなら、持ち上げる高さを返す
//
// 単位はcm。プレイヤーも敵も、同じ判定を使う

// 段差判定の設定
struct StepClimbParams
{
	// 登れる段差の最大の高さ(これより高い壁は登れない)
	float maxHeight = 50.0f;

	// この高さ未満の段差は、床の凹凸として無視し、持ち上げない
	float minHeight = 2.5f;

	// キャラクターのコライダーの半径
	float radius = 40.0f;

	// 段差の上に立った時に必要な頭上の空き(これだけ天井がなければ登れる)
	float bodyHeight = 180.0f;

	// 足元の壁(縁石)を調べるレイの、地面からの高さ。低い方から順に飛ばして、最初に当たった高さを縁石の面とする
	// 縁石の板の下が空いていて、面が地面から浮いている形状でも見つけられるよう、複数の高さを使う
	// (登れる段差の上限maxHeight以上の高さは使わない)
	int probeCount = 5;
	float probeHeights[5] = { 2.0f, 8.0f, 16.0f, 28.0f, 45.0f };

	// 足元から地面までの距離がこれ以内なら、地面にいるとみなす(接地フラグは、段差に押しつけると、角に押し上げられて外れるので使わない)
	float maxGroundClearance = 20.0f;
};

// レイの結果
struct StepRayHit
{
	bool hit = false;
	float distance = 0.0f;
	Vector3 point = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 normal = Vector3(0.0f, 0.0f, 0.0f);
};

// 段差判定の結果(なぜ登った/登らなかったか。デバッグ表示用)
enum class StepClimbResult
{
	Disabled,         // 段差を登る設定が無効
	NotGrounded,      // 接地していない(空中)
	NotMoving,        // ほとんど動いていない
	NoWallAhead,      // 足元の進行方向に壁がない(段差に触れていない)
	SlopeAhead,       // 進行方向の面が緩い坂(物理に任せる)
	WallTooTall,      // 登れる高さを超える壁
	NoTopSurface,     // 壁の上に、立てる面が見つからない
	RiseOutOfRange,   // 段差の高さが範囲外(低すぎる/高すぎる)
	Ceiling,          // 段差の上に頭上の空きがない
	Climbed,          // 登った(持ち上げた)
};

struct StepClimbDebug
{
	// 判定で飛ばしたレイ(デバッグの3D描画用。直近の判定の分)
	struct Probe
	{
		Vector3 from = Vector3(0.0f, 0.0f, 0.0f);
		Vector3 to = Vector3(0.0f, 0.0f, 0.0f);   // 当たった点。当たらなければレイの終点
		bool hit = false;
		int kind = 0;   // 0=直下の地面 1=足元の壁 2=壁の高さの探索 3=段差の上面 4=頭上
	};
	std::vector<Probe> probes;

	StepClimbResult result = StepClimbResult::Disabled;

	// 足元から直下の地面までの距離(地面に当たらなければ-1)
	float groundClearance = -1.0f;

	// 壁までの距離 / 求めた段差の高さ(範囲外でも、求められた場合は入る)
	float wallDistance = 0.0f;
	float rise = 0.0f;
};
// レイを飛ばす関数(ステージの床・壁だけが対象になるようにして渡すこと)
using StepRayFunc = std::function<StepRayHit(const Vector3& origin, const Vector3& direction, float maxDistance)>;

/// <summary>
/// 登れる段差があるなら、持ち上げる高さを返す
/// </summary>
/// <param name="pos">キャラクターの足元の座標(Transformのpos)</param>
/// <param name="moveDirection">進行方向(水平。長さは問わない)</param>
/// <param name="speed">水平の速さ(1フレームあたり)</param>
/// <param name="params">設定</param>
/// <param name="ray">レイを飛ばす関数</param>
/// <param name="debug">nullでなければ、判定の結果と理由を書き込む</param>
/// <returns>持ち上げる高さ。段差がなければ0</returns>
float ComputeStepRise(const Vector3& pos, const Vector3& moveDirection, float speed, const StepClimbParams& params, const StepRayFunc& ray, StepClimbDebug* debug = nullptr);
