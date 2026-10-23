#include "Mannequin.h"

#include "../../pch.h"

#include "../Common/Collider/CapsuleCollider.h"

#include "../Weapon/Kogetsu/Kogetsu.h"

Mannequin::Mannequin(const Vector3& pos, bool dynamicFlg)
	:CuttableActor(COLLIDER_TAG::DebugObject, true, "Data/Parameter/Mannequin/")
{
	//位置情報を設定
	trans.pos = pos;
	// 動的オブジェクトとしての処理を有効にする
	SetDynamicFlg(dynamicFlg);
}

void Mannequin::Load(void)
{
	// 切断用のMeshColliderを生成して登録する
	CuttableActor::Load();

	trans.LoadModel("Mannequin/mannequin");

	// モデルの形とテクスチャをCuttableActorへ取り込む
	// （位置・向き・大きさはこのアクターのtransで決まる）
	SetMeshFromModel(trans.model);

	// 元のモデルは描画しない（テクスチャを借りているので、削除はしない）
	MV1SetVisible(trans.model, FALSE);

	// 重力を有効にする
	SetGravityFlg(false);

	// 当たり判定による押し出しを有効にする
	SetPushFlg(true);

}

void Mannequin::SubInit(void)
{
	trans.scale = GetParameter("Init", "Scale");
}

void Mannequin::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{

}
