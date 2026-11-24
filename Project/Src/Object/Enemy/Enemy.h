#pragma once

#include <functional>
#include <vector>

#include "../Common/CharacterBase/CharacterBase.h"
#include "../Common/Effect/PolyDecompose/PolyDecompose.h"

#include "../Common/AnimationController/CharacterAnimeTypeDefine.h"

#include "Navigation/AStar.h"

class NavGrid;
class NavManager;
class EnemyChaseState;
class Kogetsu;

// 敵
// 目標(プレイヤー)を視界に入れたら、経路探索しながら追いかける
//   Idle  : 待機。視界に目標が入ったらChaseへ
//   Chase : 追跡。見失っても最後に見た位置へ向かい、一定時間見つからなければIdleへ
class Enemy : public CharacterBase
{
public:

	// 経路探索の共有情報(グリッドと探索の窓口は全員で1つを使い回す)
	struct NavContext
	{
		const NavGrid* grid;
		NavManager* manager;
	};

	// 戦闘まわりの設定(弧月と、攻撃がプレイヤーに当たった時の通知)
	struct Combat
	{
		// 複製元の弧月モデル(-1ならファイルから読み込む)
		int swordModel = -1;

	};

	/// <param name="nav">経路探索の共有情報</param>
	/// <param name="targetPos">追う目標の座標(参照先は敵より長く生きていること)</param>
	/// <param name="spawnPos">出現位置</param>
	/// <param name="baseModel">複製元のモデルハンドル(-1ならファイルから読み込む。毎回の読み込みは重いので、複製を使う)</param>
	/// <param name="peers">同じ場にいる敵の一覧(互いに離れるために使う。自分も含んでよい)</param>
	/// <param name="combat">弧月の複製元と、攻撃が当たった時の通知</param>
	Enemy(const NavContext& nav, const Vector3* targetPos, const Vector3& spawnPos, const std::vector<Enemy*>* peers, int baseModel = -1, const Combat& combat = Combat());

	~Enemy()override = default;

	void Load(void)override;

	// 描画(生成演出中は、ポリゴン分解で描く)
	void Draw(void)override;

	// 生成演出の最中か
	bool IsSpawning(void)const { return spawnEffect != nullptr; }

	// 生成演出の設定(長さ・破片の動き)を変える。シーンに追加する前に呼ぶ
	void SetSpawnParams(const PolyDecompose::Params& params) { spawnParams = params; }

	// 目標が見えているか
	bool CanSeeTarget(void)const { return canSeeTarget; }

	// 追跡中か
	bool IsChasing(void)const;

	// 倒された状態にする(実際の撤去はEnemyManagerが行う。攻撃のダメージ処理から呼ぶ想定)
	void Defeat(void) { defeated = true; }
	bool IsDefeated(void)const { return defeated; }

	// 現在位置
	Vector3 GetPos(void)const { return trans.pos; }

	// 直近のモデル準備(読み込み or 複製)にかかった時間(ミリ秒。計測用)
	static double GetLastModelLoadMs(void) { return lastModelLoadMs; }

	// デバッグ描画(視線の向き、状態、経路)
	void DrawDebug(void)const;

	void OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)override;

private:

	// 状態定義
	enum class STATE
	{
		None = -1,

		Idle,
		Chase,
		Attack,

		Max
	};

#pragma region アニメーション関係定義

	// アニメーション再生速度テーブル(プレイヤーと同じモデルを使うため同じ並び)
	static constexpr float ANIME_SPEED_TABLE[CHARACTER_ANIME::MAX] =
	{
		0.5f,	// Idle
		0.5f,	// Walk
		0.5f,	// SwordIdle.R
		0.5f,	// ShildIdle.L
		0.7f,	// SlashA1
		0.7f,	// SlashA2
		0.6f,	// SlashA3
		0.5f,	// AsteroidIdle.R
		0.5f,	// AsteroidCreate.R
		1.5f,	// AsteroidIdleShot.R
		0.5f,	// JumpStart
		0.5f,	// Jumping
		0.5f,	// JumpEnd
		0.5f,	// Run
	};

	// アニメーションループ再生フラグテーブル
	static constexpr bool ANIME_LOOP_TABLE[CHARACTER_ANIME::MAX] =
	{
		true,	// Idle
		true,	// Walk
		true,	// SwordIdle.R
		true,	// ShildIdle.L
		false,	// SlashA1
		false,	// SlashA2
		false,	// SlashA3
		true,	// AsteroidIdle.R
		true,	// AsteroidCreate.R
		true,	// AsteroidIdleShot.R
		false,	// JumpStart
		true,	// Jumping
		false,	// JumpEnd
		true,	// Run
	};

#pragma endregion

	void SubInit(void)override;
	void SubUpdate(void)override;
	void SubRelease(void)override;

	// 目標が見えているかを更新する
	void UpdatePerception(void);

	// 追跡を続けるべきか(見えている、または見失ってからまだ間もない)
	bool ShouldChase(void)const;

	// 追う目標の座標(見えていれば現在位置、見えなければ最後に見た位置)
	Vector3 GetChaseGoal(void)const;

	// 攻撃を始めてよいか(クールダウン明けで、目標が見えていて、射程内)
	bool CanStartAttack(void)const;

	// 近くにいる他の敵から離れる向き(長さ0?1)
	Vector3 ComputeSeparation(void)const;

	// 経路探索の共有情報
	const NavContext nav;

	// 目標の座標(参照のみ)
	const Vector3* targetPos;

	// 出現位置
	const Vector3 spawnPos;

	// 同じ場にいる敵の一覧(参照のみ)
	const std::vector<Enemy*>* peers;

	// 複製元のモデル(-1なら読み込む)
	const int baseModel;

	// 戦闘まわりの設定
	const Combat combat;

	static inline double lastModelLoadMs = 0.0;

	// 倒されたか
	bool defeated;

	// 追跡状態(CharacterBaseが所有する。デバッグ表示用に保持)
	EnemyChaseState* chaseState;

	// 目標が見えているか
	bool canSeeTarget;

	// 最後に目標を見た位置があるか / その位置
	bool hasLastKnownPos;
	Vector3 lastKnownPos;

	// 目標を見失ってからの経過フレーム
	float lostFrames;

	// 弧月(Actorの所有は、この敵の子アクター)
	Kogetsu* kogetsu;

	// 次に攻撃できるまでの残りフレーム
	float attackCooldown;

	// 生成演出
	PolyDecompose* spawnEffect;

	// 生成演出の経過フレーム
	float spawnFrame;

	// 生成演出の設定(初期値はPolyDecompose::Paramsの固定値)
	PolyDecompose::Params spawnParams;


	// 単位はcm(プレイヤーのコライダー半径40、全高220に合わせる)

	// 視界の距離
	const float SIGHT_RANGE = GetParameter("Init", "SightRange");

	// 視野角。追跡中は全方位を見られるものとする
	const float SIGHT_HALF_ANGLE_DEG = GetParameter("Init", "SightHalfAngleDeg");

	// 追跡中は、見失いにくいように視界の距離を伸ばす倍率
	const float CHASE_SIGHT_RANGE_RATE = GetParameter("Init", "ChaseSightRangeRate");

	// 見失ってから、最後に見た位置を探し続けるフレーム数
	const float LOSE_SIGHT_FRAMES = GetParameter("Init", "LoseSightFrames");

	// 他の敵から離れ始める距離(コライダーの直径80より少し大きい)
	const float SEPARATION_RADIUS = GetParameter("Init", "SeparationRadius");

	// 目の高さ / 目標を狙う高さ(足元からの高さ)
	const float EYE_HEIGHT = GetParameter("Init", "EyeHeight");
	const float TARGET_AIM_HEIGHT = GetParameter("Init", "TargetAimHeight");

	// 攻撃を始める距離(水平)/ 目標との高さの差の許容 / 攻撃の後の、次の攻撃までの間隔
	const float ATTACK_RANGE = GetParameter("Init", "AttackRange");
	const float ATTACK_HEIGHT_TOLERANCE = GetParameter("Init", "AttackHeightTolerance");
	const float ATTACK_COOLDOWN_FRAMES = GetParameter("Init", "AttackCooldownFrames");

	// モデルサイズ
	const float SCALE = GetParameter("Init", "Scale");


};
