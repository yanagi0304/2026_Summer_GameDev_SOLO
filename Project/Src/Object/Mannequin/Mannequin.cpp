#include "Mannequin.h"
#include "../Common/Collider/CapsuleCollider.h"
#include "../Common/MeshSlicer/MeshSlicer.h"
#include "../Weapon/Kogetsu/Kogetsu.h"

Mannequin::Mannequin()
	:ActorBase("Data/Parameter/Mannequin/")
{
}

void Mannequin::Load(void)
{
	trans.LoadModel("Mannequin/mannequin");
	// 動的オブジェクトとしての処理を有効にする
	SetDynamicFlg(true);

	// 重力を有効にする
	SetGravityFlg(true);

	// 当たり判定による押し出しを有効にする
	SetPushFlg(true);

	AddCollider(new CapsuleCollider(COLLIDER_TAG::DebugObject, GetParameterToVector3("Collider", "ColliderStartPos")
		, GetParameterToVector3("Collider", "ColliderEndPos"), GetParameter("Collider", "Radius")));

	tex = MeshSlicer::GetMaterialTextures(trans.model);     // マテリアルごとのテクスチャ

}

void Mannequin::SubInit(void)
{
	trans.scale = GetParameter("Init", "Scale");

	trans.pos = (0.0, 0.0, 200.0f);

}

void Mannequin::SubUpdate(void)
{
	if (!sliced) return;

	const float dt = 1.0f / 60.0f;

	MeshSlicer::Update(pieces[0], dt);
	MeshSlicer::Update(pieces[1], dt);
}

void Mannequin::SubDraw(void)
{
	if (!sliced) return;

	MeshSlicer::Draw(pieces[0], tex);
	MeshSlicer::Draw(pieces[1], tex);
}

void Mannequin::OnCollision(COLLIDER_TAG ownTag, const ColliderBase& other, const CollisionResult& result)
{
	if (sliced) return;                              // 二重に切らない

	// ★当たった相手のアクターを取得する関数名は、実際のクラスに合わせて直す。
	//   Kogetsu でなければ nullptr になり、剣以外との衝突は無視される。
	const Kogetsu* kogetsu = dynamic_cast<const Kogetsu*>(other.GetOwner());
	if (kogetsu == nullptr) return;

	// 剣を十分な速さで振っているときだけ切る
	const SlashInfo& info = kogetsu->GetSlashInfo();
	if (!info.valid) return;

	// 切る瞬間のワールド座標で三角形を取得する(1回だけなので負荷は問題になりにくい)
	auto tris = MeshSlicer::ExtractTriangles(trans.model);

	// 平面は「刃の向き」と「振った方向」で決まる。
	// 刃の根元は平面上にあるので、hitPoint には根元をそのまま使える。
	auto pl = MeshSlicer::MakePlane(info.base, info.bladeDir, info.slashDir);

	auto res = MeshSlicer::Slice(tris, (int)tex.size(), pl, GetColorU8(200, 30, 30, 255));
	if (!res.sliced) return;                         // 平面がモデルに当たっていなかった

	pieces[0] = res.front;
	pieces[1] = res.back;
	sliced = true;

	MV1SetVisible(trans.model, FALSE);               // 元のモデルを消す

	// ★切断後は、見えないマネキンの当たり判定が残らないようにする。
	//   ActorBase に同名の関数があれば有効(なければこの行は削除)。
	SetJudgeFlg(false);
}
