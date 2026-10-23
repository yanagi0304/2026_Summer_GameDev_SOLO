#pragma once

#include <map>
#include <vector>

#include "../ActorBase/ActorBase.h"

#include "../MeshSlicer/CutMesh.h"
#include "../MeshSlicer/MeshCutter.h"

class MeshCollider;

// 平面で切断できるアクター
// ・CutMeshを自前で描画する（モデルは持たない）
// ・同じCutMeshからMeshColliderを作る（見た目と当たり判定が一致する）
// ・Cutで2つに分け、片方を自分に残し、もう片方を新しいアクターとして返す
class CuttableActor : public ActorBase
{
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="colliderTag">MeshColliderに付けるタグ</param>
	/// <param name="dynamic">true = 動的、false = 静的（動かないマネキンやステージ）</param>
	CuttableActor(COLLIDER_TAG colliderTag, bool dynamic = false);

	/// <summary>
	/// パラメーターファイルを使う派生クラス（Mannequinなど）用のコンストラクタ
	/// </summary>
	/// <param name="parameterPath">ActorBase(const std::string&)に渡すパラメーターのパス</param>
	CuttableActor(COLLIDER_TAG colliderTag, bool dynamic, const std::string& parameterPath);

	~CuttableActor() override = default;

	// MeshColliderを生成して登録する（AddActor内で呼ばれる）
	void Load(void) override;

	// 初期化（MeshColliderをCutMeshから作り直す）
	void Init(void) override;

	/// <summary>
	/// メッシュを設定する（描画用データとMeshColliderも作り直す）
	/// 頂点はアクターのローカル座標で指定する
	/// </summary>
	void SetMesh(CutMesh newMesh);

	// 現在のメッシュ
	const CutMesh& GetMesh(void) const { return mesh; }

	/// <summary>
	/// マテリアル番号にテクスチャを割り当てる（未設定はテクスチャなし）
	/// テクスチャの寿命は呼び出し側が管理する
	/// </summary>
	void SetTexture(int materialIndex, int graphHandle);

	/// <summary>
	/// MV1モデルの現在の形とテクスチャをこのアクターへ取り込む
	/// ・モデルは「新しく読み込んだもの」を渡す（取り込み後は描画しない。このアクターが代わりに描く）
	/// ・頂点はモデル自身の座標になるので、位置・向き・大きさはこのアクターのtransで決める
	/// ・テクスチャはモデルが持つものを借りるので、モデルはこのアクターより長く残しておく
	/// </summary>
	/// <param name="modelHandle">MV1モデルのハンドル</param>
	/// <returns>取り込めたらtrue</returns>
	bool SetMeshFromModel(int modelHandle);

	/// <summary>
	/// ワールド座標の平面で切断する
	/// 平面の法線側のメッシュはこのアクターに残り、反対側は新しいアクターとして返る
	/// </summary>
	/// <param name="worldPlane">ワールド座標の切断面</param>
	/// <param name="cutId">一振りごとの斬撃ID（このアクターと断片に記録される。不要なら-1）</param>
	/// <returns>
	/// 新しく生成した断片アクター（呼び出し側でSceneBase::AddActorに渡す）
	/// 切れなかった場合はnullptr（このアクターは変化しない）
	/// </returns>
	CuttableActor* Cut(const CutPlane& worldPlane, int cutId = -1);

	// 最後に切られたときの斬撃ID（一度も切られていなければ-1）
	// 同じ一振りで、断片をもう一度切らないために使う
	int GetLastCutId(void) const { return lastCutId; }

	/// <summary>
	/// ワールド座標の平面を、このアクターのローカル座標の平面へ変換する
	/// </summary>
	CutPlane ToLocalPlane(const CutPlane& worldPlane) const;

	/// <summary>
	/// 吹き飛ばす（回転しながら重力で落ち、しばらくすると縮んで消える）
	/// 動的アクターとして生成した断片に使う
	/// </summary>
	/// <param name="launchVelocity">初速（1フレームあたりの移動量）</param>
	/// <param name="axis">回転軸（ワールド）</param>
	/// <param name="speed">回転速度（ラジアン/フレーム）</param>
	void Launch(const Vector3& launchVelocity, const Vector3& axis, float speed);

	/// <summary>
	/// 残っている割合を取得する（切る前の表面積に対する、今の表面積）
	/// ・切るたびに減っていく
	/// ・断片は「元のモデル全体に対する自分の取り分」になる
	/// </summary>
	/// <returns>0.0（何も残っていない）〜 1.0（切られていない）</returns>
	float GetRemainRate(void) const;

	/// <summary>
	/// 損傷率を取得する（1.0 - 残っている割合）
	/// </summary>
	/// <returns>0.0（無傷）〜 1.0（全部切れた）</returns>
	float GetDamageRate(void) const { return 1.0f - GetRemainRate(); }

	// 飛び終わって消えたか（シーンから外してよい状態か）
	bool IsVanished(void) const { return vanished; }

	// 飛ばさずに、その場で縮めて消す（残りがごくわずかになった本体用）
	void StartVanish(void);

protected:

	// 飛んでいる間の更新（回転・縮小・消滅）
	void SubUpdate(void) override;

	// メッシュの描画
	void SubDraw(void) override;

	// 解放
	void SubRelease(void) override;

private:

	// 1マテリアル分の描画データ
	struct DrawBatch
	{
		// 元モデルのマテリアル番号
		int materialIndex;

		// 頂点数が65535以下のとき使う16bitインデックス
		std::vector<unsigned short> indices16;

		// 頂点数が多く16bitで足りないとき、三角形ごとに頂点を展開して持つ
		std::vector<VERTEX3D> expanded;

		DrawBatch(void) : materialIndex(0), indices16(), expanded() {}
	};

	// 16bitインデックスで指せる最大の頂点数
	static constexpr size_t INDEX16_VERTEX_MAX = 65535;

	// これより残りが少なくなったら消す（元の表面積に対する割合）
	static constexpr float MIN_REMAIN_RATE = 0.05f;

	// MeshColliderに付けるタグ
	COLLIDER_TAG colliderTag;

	// 切断対象のメッシュ（ローカル座標）
	CutMesh mesh;

	// マテリアル番号 -> テクスチャハンドル
	std::map<int, int> textures;

	// 当たり判定（所有はActorBase側）
	MeshCollider* meshCollider;

	// 描画用の頂点配列（インデックス描画で使う）
	std::vector<VERTEX3D> drawVertices;

	// マテリアルごとの描画データ
	std::vector<DrawBatch> batches;

	// 最後に切られたときの斬撃ID
	int lastCutId;

	// 切った断片が飛ぶ強さ（切断面の法線の反対方向へ）
	static constexpr float PUSH_SPEED = 6.0f;

	// 切った断片が最初に上へ跳ねる強さ
	static constexpr float PUSH_UP = 10.0f;

	// 切った断片の回転速度（ラジアン/フレーム）
	static constexpr float SPIN_SPEED = 0.04f;

	// 飛ばしてから縮み始めるまでの時間（フレーム）
	static constexpr float SHRINK_START_TIME = 40.0f;

	// 飛ばしてから完全に消えるまでの時間（フレーム）
	static constexpr float VANISH_TIME = 90.0f;

	// 飛んでいる最中か
	bool flying;

	// 飛ばしてからの経過時間（フレーム。TimeScale反映）
	float flyTime;

	// 飛ばした時点のスケール（縮小の基準）
	Vector3 launchScale;

	// 回転軸（ワールド）と回転速度
	Vector3 spinAxis;
	float spinSpeed;

	// 読み込み時点の表面積（損傷率の基準。フタは含めない）
	float originalArea;

	bool vanished;

	// メッシュを自分の中心が原点になるようずらし、ずらした量（ローカル）を返す
	// （断片が元の箱の中心ではなく、自分の中心で回るようにするため）
	static Vector3 RecenterMesh(CutMesh& target);

	// MeshColliderをmeshから作り直す
	void RebuildCollider(void);

	// 描画用データをmeshから作り直す
	void BuildDrawBatches(void);

	// マテリアル番号のテクスチャを取得（無ければテクスチャなし）
	int GetTexture(int materialIndex) const;
};
