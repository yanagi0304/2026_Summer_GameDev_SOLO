#include "Kogetsu.h"

#include "../../../pch.h"

#include "../../../Utility/Utility.h"

#include "../../Common/Collider/CapsuleCollider.h"

#include "../../Common/MeshSlicer/CutRequestQueue.h"

#include "../../Common/CuttableActor/CuttableActor.h"

#include "../../Common/Shader/RimLightShader.h"

#include "../../Common/Effect/PolyDecompose/PolyDecompose.h"

#include "../../../Manager/TimeScale/TimeScale.h"

Kogetsu::Kogetsu()
	:
	WeaponBase(Transform(), "Data/Parameter/Weapon/Kogetsu/"),
	isSheathed(false),
	baseModel(-1)
{
}

Kogetsu::Kogetsu(const Transform& ownerTrans, int baseModel, const DecreaseTrionFunc& decreaseTrion)
	:
	WeaponBase(ownerTrans, "Data/Parameter/Weapon/Kogetsu/", decreaseTrion),
	attackColl(new CapsuleCollider(COLLIDER_TAG::Kogetsu,
		BladeDirHand()* BLADE_START, BladeDirHand()* (BLADE_START + BLADE_LENGTH), BLADE_RADIUS)),
	isSheathed(false),
	baseModel(baseModel)
{
}

void Kogetsu::Load(void)
{
	// 複製元があれば複製する(ファイルからの読み込みは重い)
	if (baseModel >= 0) { ModelDuplicate(baseModel); }
	else { trans.LoadModel("Weapon/Kogetsu"); }

	trans.SetLocalRotation(Quaternion::FromRotationZ(Deg2Rad(-90.0f)));

	SetJudgeFlg(false);

	// 動的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Movable);

	attackColl->SetJudgeFlg(false);

	AddCollider(attackColl);

	// 当たり判定による押し出しを有効にする
	SetContactResponseEnabled(false);
	
	// シェーダーを差し替える
	CreateShader(new RimLightShader(SHADER_VERTEX_TYPE::Frame1));

	SetIsDraw(false);


}

void Kogetsu::On(void)
{
	attackColl->SetJudgeFlg(true);
	slash_.Reset();
	cutId_ = CutRequestQueue::IssueCutId();   // 一振りごとに新しいID
}

void Kogetsu::Off(void)
{
	attackColl->SetJudgeFlg(false);
}

void Kogetsu::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other,
	const CollisionResult& result)
{
	if (other.GetTag() != COLLIDER_TAG::Mannequin) { return; }
	CuttableActor* target = dynamic_cast<CuttableActor*>(other.GetActor());
	if (target == nullptr) { return; }
	// この一振りで切った相手は除く
	if (target->GetLastCutId() == cutId_) { return; }

	CutPlane plane;
	// 突き・停止中は見送る
	if (!slash_.GetPlane(plane)) { return; }
	// リクエストをキューに追加する
	CutRequestQueue::Push(target, plane, cutId_);
}

void Kogetsu::OnEquip(void)
{
	SetJudgeFlg(false);
	// 動的オブジェクトとしての処理を有効にする
	SetColliderMobility(COLLIDER_MOBILITY::Movable);
	
}

void Kogetsu::SubInit(void)
{
	trans.scale = Vector3(GetParameter("Init", "scale"));

	// 生成演出の準備
	if (spawnParams.frames > 0.0f)
	{
		trans.Attach();
		spawnEffect = new PolyDecompose();
		if (!spawnEffect->Init(trans.model))
		{
			// 準備できなければ演出なしで出す
			spawnEffect->Release();
			delete spawnEffect;
			spawnEffect = nullptr;
		}
	}
}

namespace
{
	// 行列からスケールを取り除いた回転行列を返す(Rot:ロット=回転)
	MATRIX GetRotationWithoutScale(const MATRIX& mat)
	{
		const VECTOR scale = MGetSize(mat);
		return MMult(
			MGetRotElem(mat),
			MGetScale(VGet(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z))
		);
	}
}

// 更新処理: 右手に追従し、刀身の軌跡を更新する
void Kogetsu::SubUpdate(void)
{
	// 生成演出を進める。終わったら通常の描画に切り替える
	if (spawnEffect != nullptr)
	{
		spawnFrame += TimeScale::Get();
		if (spawnFrame >= spawnParams.frames)
		{
			spawnEffect->Release();
			delete spawnEffect;
			spawnEffect = nullptr;
		}
	}

	// ボーンが見つからないときは何もしない
	if (!FollowBone()) { return; }

	// 納刀中は斬撃を作らない
	if (!isSheathed) { UpdateSlash(); }
}

// 右手ボーンに合わせて、座標と回転を更新する(ボーンが無ければ false)
bool Kogetsu::FollowBone(void)
{
	// 追従するボーン(Frame:フレーム=モデルのボーン)を探す
	const char* boneName = isSheathed ? WAIST_BONE_NAME : HAND_BONE_NAME;
	const int frameIndex = MV1SearchFrame(ownerTrans.model, boneName);
	if (frameIndex == -1) { return false; }

	// ボーンのワールド行列(DxLibの関数がconstなし参照を受けるので const を付けない)
	MATRIX boneMat = MV1GetFrameLocalWorldMatrix(ownerTrans.model, frameIndex);

	// スケールを除いた回転行列
	const MATRIX boneRotationMat = GetRotationWithoutScale(boneMat);

	// 座標: ボーンの位置 + 回転だけで変換したずれ
	const Vector3 offset = isSheathed ? SHEATH_OFFSET : HAND_OFFSET;
	trans.pos =
		Vector3(MGetTranslateElem(boneMat)) +
		Vector3(VTransform(offset.ToVECTOR(), boneRotationMat));

	// 回転: ボーンの向き(納刀中は、さらに腰用の回転を加える)
	const Quaternion boneRotation = Quaternion::FromMatrix(boneRotationMat);
	trans.SetRotation(isSheathed ? boneRotation * SheathRotation() : boneRotation);

	return true;
}

// 刀身の根元と先端から、斬撃の軌跡を更新する
void Kogetsu::UpdateSlash(void)
{
	const Vector3 bladeDir = trans.VTrans(BladeDirHand());
	slash_.Update(
		trans.pos + bladeDir * BLADE_START,
		trans.pos + bladeDir * (BLADE_START + BLADE_LENGTH)
	);
}

void Kogetsu::SubDraw(void)
{
//#ifdef _DEBUG
//	const Vector3 o = trans.pos;
//	DrawLine3D(o.ToVECTOR(), (o + trans.VTrans(Vector3::Xonly(1.0f)) * 100.0f).ToVECTOR(), GetColor(255, 0, 0));     // X = 赤
//	DrawLine3D(o.ToVECTOR(), (o + trans.VTrans(Vector3::Yonly(1.0f)) * 100.0f).ToVECTOR(), GetColor(255, 255, 0));   // Y = 黄
//	DrawLine3D(o.ToVECTOR(), (o + trans.VTrans(Vector3::Zonly(1.0f)) * 100.0f).ToVECTOR(), GetColor(255, 255, 255)); // Z = 白
//#endif
}

void Kogetsu::SubRelease(void)
{
	if (spawnEffect != nullptr)
	{
		spawnEffect->Release();
		delete spawnEffect;
		spawnEffect = nullptr;
	}
}

void Kogetsu::Draw(void)
{
	// 通常は、そのままモデルを描く
	if (spawnEffect == nullptr)
	{
		WeaponBase::Draw();
		return;
	}

	if (!GetIsDraw()) { return; }

	// 生成演出中は、三角形の破片が組み上がる(進行度は1→0)
	trans.Attach();
	spawnEffect->Update();
	spawnEffect->Draw(1.0f - spawnFrame / spawnParams.frames, spawnParams);
}