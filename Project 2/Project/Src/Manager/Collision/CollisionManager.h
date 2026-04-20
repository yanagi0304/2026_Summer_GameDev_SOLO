#pragma once

#include<vector>

#include"../../Object/Common/Transform.h"

#include"../../Object/Common/Collider/LineCollider.h"
#include"../../Object/Common/Collider/SphereCollider.h"
#include"../../Object/Common/Collider/CapsuleCollider.h"
#include"../../Object/Common/Collider/BoxCollider.h"
#include"../../Object/Common/Collider/ModelCollider.h"

class CollisionManager
{
public:
	CollisionManager()
		//playerColliders_(),
		//enemyColliders_(),
		//stageColliders_()
	{}
	~CollisionManager() {}

	// オブジェクト追加
	static void Add(ColliderBase* collider);
	// オブジェクト追加
	static void Add(std::vector<ColliderBase*> collider) { for (ColliderBase*& c : collider) { Add(c); } }

	// 判定実行
	void Check(void);

	// 解放
	void Clear(void) {
		playerColliders_.clear();
		enemyColliders_.clear();
		stageColliders_.clear();
	}

private:
#pragma region タイプ別コライダー格納配列
	// プレイヤー系
	static std::vector<ColliderBase*>playerColliders_;
	// エネミー系	
	static std::vector<ColliderBase*>enemyColliders_;
	// ステージ系
	static std::vector<ColliderBase*>stageColliders_;

	// それ以外
	static std::vector<ColliderBase*>otherColliders_;

	/*以下の組み合わせの判定が実行される（順番も以下の通り）
	* 
	* ①プレイヤー系×ステージ系
	* ②エネミー系×ステージ系
	* ③それ以外×ステージ系
	* 
	* ④プレイヤー系×エネミー系
	* ⑤プレイヤー系×それ以外
	* ⑥エネミー系×それ以外
	* 
	* ⑦それ以外×それ以外
	*/
#pragma endregion

#pragma region 当たり判定用
	void Matching(std::vector<ColliderBase*>& as, std::vector<ColliderBase*>& bs);
	void Matching(std::vector<ColliderBase*>& s);
	bool IsHit(ColliderBase* a, ColliderBase* b);

	bool LineToLine(LineCollider* a, LineCollider* b);
	bool SphereToSphere(SphereCollider* a, SphereCollider* b);
	bool CapsuleToCapsule(CapsuleCollider* a, CapsuleCollider* b);
	bool BoxToBox(BoxCollider* a, BoxCollider* b);
	bool ModelToModel(ModelCollider* a, ModelCollider* b);

	bool LineToSphere(LineCollider* line, SphereCollider* sphere);
	bool LineToCapsule(LineCollider* line, CapsuleCollider* capsule);
	bool LineToBox(LineCollider* line, BoxCollider* box);
	bool LineToModel(LineCollider* line, ModelCollider* model);
	bool SphereToCapsule(SphereCollider* sphere, CapsuleCollider* capsule);
	bool SphereToBox(SphereCollider* sphere, BoxCollider* box);
	bool SphereToModel(SphereCollider* sphere, ModelCollider* model);
	bool CapsuleToBox(CapsuleCollider* capsule, BoxCollider* box);
	bool CasuleToModel(CapsuleCollider* capsule, ModelCollider* model);
	bool BoxToModel(BoxCollider* box, ModelCollider* model);

#pragma endregion

#pragma region ユーティリティ
	/// <summary>
	/// 指定した２つのコライダー同士に押し出し処理が必要かどうか
	/// </summary>
	/// <param name="a">コライダー１</param>
	/// <param name="b">コライダー２</param>
	/// <returns></returns>
	bool NeedPush(ColliderBase* a, ColliderBase* b)const {
		if (a->GetPushFlg() && b->GetPushFlg()) { return true; }
		return false;
	}

	/// <summary>
	/// 重みづけの割合を計算
	/// </summary>
	/// <param name="aWeight">（in） 重み</param>
	/// <param name="bWeight">（in） 重み</param>
	/// <param name="aWeightRatio">（out） 重みの割合</param>
	/// <param name="bWeightRatio">（out） 重みの割合</param>
	void WeightRatioCalculation(unsigned char aWeight, unsigned char bWeight, float& aWeightRatio, float& bWeightRatio) {
		// お互いの重みにおける割合を計算（相手の重み ÷ 自分と相手の重みの合計）

		// 自分と相手の重みの合計
		float abWeightSum = (float)(aWeight + bWeight);

		if (abWeightSum != 0.0f) {
			aWeightRatio = (float)bWeight / abWeightSum;
			bWeightRatio = (float)aWeight / abWeightSum;
		}
		else {
			// 両方重み０だったら半分ずつで返す（例外処理）
			aWeightRatio = bWeightRatio = 0.5f;
		}
	}

	/// <summary>
	/// 押し出し処理
	/// </summary>
	/// <param name="a">コライダー１</param>
	/// <param name="b">コライダー２</param>
	/// <param name="normal">押し出し方向</param>
	/// <param name="overlap">めり込んだ量</param>
	void ApplyPush(ColliderBase* a, ColliderBase* b, const Vector3& normal, float overlap) {
		// 動的フラグ
		bool aDynamic = a->GetDynamicFlg();
		bool bDynamic = b->GetDynamicFlg();

		// 両方動的オブジェクトの場合
		if (aDynamic && bDynamic)
		{
			float aRatio = 0.0f, bRatio = 0.0f;
			WeightRatioCalculation(a->GetPushWeight(), b->GetPushWeight(), aRatio, bRatio);

			a->SetTransformPos(a->GetTransform().pos + normal * (overlap * aRatio));
			b->SetTransformPos(b->GetTransform().pos - normal * (overlap * bRatio));

			if (normal.y > 0.5f) { a->CallOnGrounded(); }
			else if (normal.y < -0.5f) { b->CallOnGrounded(); }
		}
		// aだけ動的の場合
		else if (aDynamic && !bDynamic)
		{
			a->SetTransformPos(a->GetTransform().pos + normal * overlap);
			if (normal.y > 0.5f) { a->CallOnGrounded(); }
		}
		// bだけ動的の場合
		else if (!aDynamic && bDynamic)
		{
			b->SetTransformPos(b->GetTransform().pos - normal * overlap);
			if (normal.y < -0.5f) { b->CallOnGrounded(); }
		}
		// 両方静的オブジェクトの場合
		else { /*何もしない*/ }
	}

	/// <summary>
	/// 押し出し処理
	/// </summary>
	/// <param name="a">コライダー１</param>
	/// <param name="b">コライダー２</param>
	/// <param name="overlapVec">押し出しベクトル</param>
	void ApplyPush(ColliderBase* a, ColliderBase* b, const Vector3& overlapVec) {
		// 動的フラグ
		bool aDynamic = a->GetDynamicFlg();
		bool bDynamic = b->GetDynamicFlg();

		// 両方動的オブジェクトの場合
		if (aDynamic && bDynamic)
		{
			float aRatio = 0.0f, bRatio = 0.0f;
			WeightRatioCalculation(a->GetPushWeight(), b->GetPushWeight(), aRatio, bRatio);

			a->SetTransformPos(a->GetTransform().pos + overlapVec * aRatio);
			b->SetTransformPos(b->GetTransform().pos - overlapVec * bRatio);

			Vector3 normalOverlapVec = overlapVec.Normalized();

			if (normalOverlapVec.y > 0.5f) { a->CallOnGrounded(); }
			else if (normalOverlapVec.y < -0.5f) { b->CallOnGrounded(); }
		}
		// aだけ動的の場合
		else if (aDynamic && !bDynamic)
		{
			a->SetTransformPos(a->GetTransform().pos + overlapVec);
			if (overlapVec.Normalized().y > 0.5f) { a->CallOnGrounded(); }
		}
		// bだけ動的の場合
		else if (!aDynamic && bDynamic)
		{
			b->SetTransformPos(b->GetTransform().pos - overlapVec);
			if (overlapVec.Normalized().y < -0.5f) { b->CallOnGrounded(); }
		}
		// 両方静的オブジェクトの場合
		else { /*何もしない*/ }
	}

	/// <summary>
	/// 押し出し処理
	/// </summary>
	/// <param name="a">無条件に押し出される方</param>
	/// <param name="b">無条件に動かさず押し出す方</param>
	/// <param name="overlapVec">押し出しベクトル</param>
	void ApplyPushOneSide(ColliderBase* dynamicColl, ColliderBase* staticColl, const Vector3& overlapVec) {
		dynamicColl->SetTransformPos(dynamicColl->GetTransform().pos + overlapVec);
		if (overlapVec.Normalized().y > 0.5f) { dynamicColl->CallOnGrounded(); }
	}
#pragma endregion

};