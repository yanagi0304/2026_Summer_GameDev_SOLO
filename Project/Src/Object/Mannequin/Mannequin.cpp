#include "Mannequin.h"

#include "../../pch.h"

#include "../Common/Collider/CapsuleCollider.h"

#include "../Weapon/Kogetsu/Kogetsu.h"

Mannequin::Mannequin()
	:CuttableActor(COLLIDER_TAG::DebugObject, true, "Data/Parameter/Mannequin/")
{
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

	// 動的オブジェクトとしての処理を有効にする
	SetDynamicFlg(true);

	// 重力を有効にする
	SetGravityFlg(true);

	// 当たり判定による押し出しを有効にする
	SetPushFlg(true);

}

void Mannequin::SubInit(void)
{
	trans.scale = GetParameter("Init", "Scale");

	trans.pos = Vector3(0.0f, 0.0f, 200.0f);

}

void Mannequin::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{

}
