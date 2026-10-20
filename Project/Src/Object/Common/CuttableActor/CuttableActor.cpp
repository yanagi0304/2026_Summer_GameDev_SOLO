#include "CuttableActor.h"

#include <algorithm>
#include <utility>

#include "../../../pch.h"

#include "../../../Manager/TimeScale/TimeScale.h"

#include "../MeshSlicer/CutMeshBuilder.h"

#include "../Collider/MeshCollider.h"

#include "../MeshSlicer/MeshCutter.h"

namespace
{
	// 0.0～1.0 の色成分を 0～255 に変換する
	int ToByte(float value)
	{
		const float clamped = (std::max)(0.0f, (std::min)(1.0f, value));
		return static_cast<int>(clamped * 255.0f + 0.5f);
	}
}

CuttableActor::CuttableActor(COLLIDER_TAG colliderTag, bool dynamic) :
	ActorBase(),

	colliderTag(colliderTag),

	mesh(),
	textures(),

	meshCollider(nullptr),

	drawVertices(),
	batches(),

	lastCutId(-1),

	flying(false),
	flyTime(0.0f),
	launchScale(1.0f),
	spinAxis(0.0f, 1.0f, 0.0f),
	spinSpeed(0.0f)
{
	SetDynamicFlg(dynamic);
}

CuttableActor::CuttableActor(COLLIDER_TAG colliderTag, bool dynamic, const std::string& parameterPath) :
	ActorBase(parameterPath),

	colliderTag(colliderTag),

	mesh(),
	textures(),

	meshCollider(nullptr),

	drawVertices(),
	batches(),

	lastCutId(-1),

	flying(false),
	flyTime(0.0f),
	launchScale(1.0f),
	spinAxis(0.0f, 1.0f, 0.0f),
	spinSpeed(0.0f)
{
	SetDynamicFlg(dynamic);
}

void CuttableActor::Load(void)
{
	// 二重Load防止（コライダーが二重登録されるため）
	if (meshCollider != nullptr) { return; }

	meshCollider = new MeshCollider(colliderTag);

	AddCollider(meshCollider);
}

void CuttableActor::Init(void)
{
	ActorBase::Init();

	// ActorBase::Init内のMeshCollider::Initは、モデルが無いと中身を空にしてしまう
	// そのため、Initが終わった後に自前のメッシュから作り直す
	RebuildCollider();
}

void CuttableActor::SetMesh(CutMesh newMesh)
{
	mesh = std::move(newMesh);

	BuildDrawBatches();

	RebuildCollider();
}

void CuttableActor::SetTexture(int materialIndex, int graphHandle)
{
	textures[materialIndex] = graphHandle;
}

bool CuttableActor::SetMeshFromModel(int modelHandle)
{
	CutMesh converted;

	if (!CutMeshBuilder::FromModel(modelHandle, converted)) { return false; }

	SetMesh(std::move(converted));

	// マテリアルごとに、モデルが持つ拡散色テクスチャを割り当てる
	const int materialNum = MV1GetMaterialNum(modelHandle);

	for (int i = 0; i < materialNum; i++) {

		const int textureIndex = MV1GetMaterialDifMapTexture(modelHandle, i);
		if (textureIndex < 0) { continue; }

		const int graph = MV1GetTextureGraphHandle(modelHandle, textureIndex);
		if (graph < 0) { continue; }

		SetTexture(i, graph);
	}

	return true;
}

CutPlane CuttableActor::ToLocalPlane(const CutPlane& worldPlane) const
{
	// MeshColliderのToLocalPointと同じワールド行列を使う
	const MATRIX worldMat = trans.GetWorldMatrix();

	// 点：ワールド -> ローカルは逆行列
	const Vector3 localPoint = Vector3(VTransform(
		worldPlane.point.ToVECTOR(),
		MInverse(worldMat)
	));

	// 法線：ワールド行列の転置で変換する
	// （回転だけなら逆回転、スケールが不均一でも面の向きが崩れない）
	const Vector3 localNormal = Vector3(VTransformSR(
		worldPlane.normal.ToVECTOR(),
		MTranspose(worldMat)
	));

	return CutPlane(localPoint, localNormal);
}

CuttableActor* CuttableActor::Cut(const CutPlane& worldPlane, int cutId)
{
	if (mesh.IsEmpty()) { return nullptr; }

	const CutPlane localPlane = ToLocalPlane(worldPlane);

	CutMesh front;
	CutMesh back;

	// 片側にしか三角形が無ければ切れていない
	if (!MeshCutter::Cut(mesh, localPlane, front, back)) { return nullptr; }

	// 裏側を新しいアクターにする（飛ばすので必ず動的）
	CuttableActor* fragment = new CuttableActor(colliderTag, true);

	// 位置・回転・スケールを引き継ぐ
	fragment->trans = trans;

	// 断片の中心が原点になるようメッシュをずらし、見た目が変わらないよう座標を補正する
	const Vector3 localCenter = RecenterMesh(back);
	{
		const MATRIX worldMat = trans.GetWorldMatrix();
		const Vector3 originWorld = Vector3(VTransform(VGet(0.0f, 0.0f, 0.0f), worldMat));
		const Vector3 centerWorld = Vector3(VTransform(localCenter.ToVECTOR(), worldMat));
		fragment->trans.pos = trans.pos + (centerWorld - originWorld);
	}


	// このアクターはモデルを持たない。ハンドルを二重に解放しないよう念のため-1にする
	fragment->trans.model = -1;

	fragment->textures = textures;

	// 断片にも同じ斬撃IDを記録する（同じ一振りで切り直されないようにする）
	fragment->lastCutId = cutId;

	// 飛んでいく断片がプレイヤーを押し出さないようにする
	fragment->SetPushFlg(false);
	fragment->SetPushWeight(GetPushWeight());
	fragment->SetDrawType(GetDrawType());

	fragment->SetMesh(std::move(back));

	// 裏側（法線の反対）へ飛ばす。回転軸は、法線と上方向に垂直な軸
	{
		const Vector3 up(0.0f, 1.0f, 0.0f);
		Vector3 axis = worldPlane.normal.Cross(up);
		if (axis.Length() < 0.001f) { axis = Vector3(1.0f, 0.0f, 0.0f); }

		const Vector3 launchVelocity = worldPlane.normal * -PUSH_SPEED + up * PUSH_UP;

		fragment->Launch(launchVelocity, axis, SPIN_SPEED);
	}

	// 表側は自分に残す
	SetMesh(std::move(front));
	lastCutId = cutId;

	return fragment;
}

void CuttableActor::Launch(const Vector3& launchVelocity, const Vector3& axis, float speed)
{
	velocity = launchVelocity;
	spinAxis = axis;
	spinSpeed = speed;

	// 水平方向の速度が毎フレーム減らされないようにする
	DECEL_RATE = 0.0f;

	SetGravityFlg(true);

	launchScale = trans.scale;
	flyTime = 0.0f;
	flying = true;
}

void CuttableActor::SubUpdate(void)
{
	if (!flying) { return; }

	flyTime += TimeScale::Get();

	// 回転
	if (spinSpeed != 0.0f) {

		const Quaternion step = Quaternion::FromAxisAngle(spinAxis, spinSpeed * TimeScale::Get());

		trans.SetRotation((step * trans.rotation).Normalized());
	}

	// 縮み始めの時間を過ぎたら、消えるまで線形に縮める
	if (flyTime >= SHRINK_START_TIME) {

		const float t = (flyTime - SHRINK_START_TIME) / (VANISH_TIME - SHRINK_START_TIME);
		const float ratio = (std::max)(0.0f, 1.0f - t);

		trans.scale = launchScale * ratio;
	}

	// 消える時間になったら、描画と判定を止める
	if (flyTime >= VANISH_TIME) {

		flying = false;

		velocity = Vector3();
		SetGravityFlg(false);

		SetIsDraw(false);
		SetJudgeFlg(false);
	}
}

Vector3 CuttableActor::RecenterMesh(CutMesh& target)
{
	if (target.vertices.empty()) { return Vector3(); }

	Vector3 minPos = target.vertices[0].pos;
	Vector3 maxPos = target.vertices[0].pos;

	for (const CutVertex& v : target.vertices) {

		minPos.x = (std::min)(minPos.x, v.pos.x);
		minPos.y = (std::min)(minPos.y, v.pos.y);
		minPos.z = (std::min)(minPos.z, v.pos.z);

		maxPos.x = (std::max)(maxPos.x, v.pos.x);
		maxPos.y = (std::max)(maxPos.y, v.pos.y);
		maxPos.z = (std::max)(maxPos.z, v.pos.z);
	}

	const Vector3 center = (minPos + maxPos) * 0.5f;

	for (CutVertex& v : target.vertices) { v.pos = v.pos - center; }

	return center;
}

void CuttableActor::SubDraw(void)
{
	if (batches.empty()) { return; }

	// MeshColliderと同じワールド行列で描画する（見た目と当たり判定を一致させる）
	MATRIX worldMat = trans.GetWorldMatrix();
	SetTransformToWorld(&worldMat);

	for (DrawBatch& batch : batches) {

		const int texture = GetTexture(batch.materialIndex);

		if (!batch.indices16.empty()) {

			DrawPolygonIndexed3D(
				drawVertices.data(),
				static_cast<int>(drawVertices.size()),
				batch.indices16.data(),
				static_cast<int>(batch.indices16.size() / 3),
				texture,
				TRUE
			);
		}
		else if (!batch.expanded.empty()) {

			DrawPolygon3D(
				batch.expanded.data(),
				static_cast<int>(batch.expanded.size() / 3),
				texture,
				TRUE
			);
		}
	}

	// 変換行列を元に戻す（他の描画へ影響させない）
	MATRIX identMat = MGetIdent();
	SetTransformToWorld(&identMat);
}

void CuttableActor::SubRelease(void)
{
	// コライダー本体はActorBase::Releaseで解放される
	meshCollider = nullptr;

	mesh.Clear();
	textures.clear();
	drawVertices.clear();
	batches.clear();
}

void CuttableActor::RebuildCollider(void)
{
	// Load前（コライダー未生成）なら、Init後に作り直すので何もしない
	if (meshCollider == nullptr) { return; }

	std::vector<Vector3> positions;
	std::vector<unsigned int> indices;
	mesh.ToPositionsAndIndices(positions, indices);

	// 三角形が無いときはfalseが返り、コライダーは空になる
	meshCollider->Setup(positions, indices);
}

void CuttableActor::BuildDrawBatches(void)
{
	drawVertices.clear();
	batches.clear();

	if (mesh.vertices.empty()) { return; }

	// 頂点配列を作る
	drawVertices.resize(mesh.vertices.size());

	for (size_t i = 0; i < mesh.vertices.size(); i++) {

		const CutVertex& src = mesh.vertices[i];
		VERTEX3D& dst = drawVertices[i];

		dst.pos = src.pos.ToVECTOR();
		dst.norm = src.normal.ToVECTOR();

		dst.dif = GetColorU8(ToByte(src.color.r), ToByte(src.color.g), ToByte(src.color.b), ToByte(src.color.a));
		dst.spc = GetColorU8(0, 0, 0, 0);

		dst.u = src.u;
		dst.v = src.v;

		dst.su = 0.0f;
		dst.sv = 0.0f;
	}

	// 16bitインデックスで頂点を指せるか
	const bool useIndex16 = mesh.vertices.size() <= INDEX16_VERTEX_MAX;

	// マテリアルごとに描画データを作る
	for (const CutSubMesh& sub : mesh.subMeshes) {

		if (sub.indices.size() < 3) { continue; }

		DrawBatch batch;
		batch.materialIndex = sub.materialIndex;

		const size_t indexNum = sub.indices.size() / 3 * 3;

		if (useIndex16) {
			batch.indices16.reserve(indexNum);
			for (size_t i = 0; i < indexNum; i++) {
				batch.indices16.push_back(static_cast<unsigned short>(sub.indices[i]));
			}
		}
		else {
			batch.expanded.reserve(indexNum);
			for (size_t i = 0; i < indexNum; i++) {
				batch.expanded.push_back(drawVertices[sub.indices[i]]);
			}
		}

		batches.push_back(std::move(batch));
	}
}

int CuttableActor::GetTexture(int materialIndex) const
{
	const auto it = textures.find(materialIndex);

	return (it != textures.end()) ? it->second : DX_NONE_GRAPH;
}
