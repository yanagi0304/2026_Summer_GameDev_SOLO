#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "../../../Common/Vector3.h"

// ロックオンできる対象
struct LockCandidate
{
	// 対象ごとの通し番号(倒れた対象の番号は使い回さない)
	int id;

	// 狙う座標
	Vector3 pos;
};

// カメラの正面に一番近い対象を、自動でロックオンする
class LockOnSystem
{
public:

	// いまロックオンできる対象を、一覧へ追加する関数
	using CollectFunc = std::function<void(std::vector<LockCandidate>&)>;

	LockOnSystem(void) = default;
	~LockOnSystem(void) = default;

	// 対象の一覧を作る関数を設定する
	void SetCandidateSource(const CollectFunc& collect) { collect_ = collect; }

	// 毎フレーム呼ぶ
	// selecting: falseなら、座標の更新だけしてロックオンはしない
	void Update(bool selecting);

	// ロックオン中の対象にマーカーを描く(3Dのカメラ設定が有効な間に呼ぶ)
	void DrawMarker(void) const;

	// ロックオン中の対象の番号(いなければstd::nullopt)
	std::optional<int> GetTargetId(void) const { return targetId_; }

	// 番号から対象の座標を取得する(倒れていればstd::nullopt)
	std::optional<Vector3> GetPos(int id) const;

private:

	// ロックオンする対象を選ぶ
	std::optional<int> SelectTarget(void) const;

	// カメラと対象の間を、ステージが遮っているか
	bool IsBlocked(const Vector3& from, const Vector3& to) const;

private:

	// ロックオンできる最大距離(cm)
	static constexpr float LOCK_RANGE = 5000.0f;

	// カメラの正面から、この角度以内の対象だけをロックオンする(度)
	static constexpr float LOCK_ANGLE_DEG = 20.0f;

	// マーカーの半径と線の太さ(ピクセル)
	static constexpr int MARKER_RADIUS = 24;
	static constexpr int MARKER_THICKNESS = 3;

	// 対象の一覧を作る関数
	CollectFunc collect_;

	// このフレームの対象の一覧
	std::vector<LockCandidate> candidates_;

	// ロックオン中の対象の番号
	std::optional<int> targetId_;
};
