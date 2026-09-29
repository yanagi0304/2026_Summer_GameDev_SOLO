#include "Transform.h"

#include"../../../Utility/Utility.h"

Vector3 Transform::Velocity(void) const { return (pos - prevPos); }

MATRIX Transform::AngleMat(void) const { return MatrixAllMultXZY({ angle }); }

Vector3 Transform::VTrans(const Vector3& v)const { return (v != 0.0f) ? Vector3(VTransform(v.ToVECTOR(), AngleMat())) : Vector3(); }

Vector3 Transform::VTrans(const VECTOR& v) const { return VTrans(Vector3(v)); }

void Transform::SetAngleDeg(const Vector3& deg) { angle = Deg2Rad(deg); }
void Transform::SetAngleXDeg(float deg) { angle.x = Deg2Rad(deg); }
void Transform::SetAngleYDeg(float deg) { angle.y = Deg2Rad(deg); }
void Transform::SetAngleZDeg(float deg) { angle.z = Deg2Rad(deg); }

void Transform::AddAngleDeg(const Vector3& deg) { angle += Deg2Rad(deg); }
void Transform::AddAngleXDeg(float deg) { angle.x += Deg2Rad(deg); }
void Transform::AddAngleYDeg(float deg) { angle.y += Deg2Rad(deg); }
void Transform::AddAngleZDeg(float deg) { angle.z += Deg2Rad(deg); }

void Transform::LoadModel(std::string path) { model = MV1LoadModel(("Data/Model/" + path + ".mv1").c_str()); }

void Transform::Duplicate(int model) { this->model = MV1DuplicateModel(model); }

void Transform::LoadEffect(std::string path) { model = LoadEffekseerEffect(("Data/Effect/" + path + ".efk").c_str()); }

void Transform::Attach(void) {

	if (model == -1) { return; }

	MV1ModelMatrix(
		model,
		scale,
		pos + centerDiff.TransMat(MatrixAllMultZXY({ localAngle,angle })),
		{ localAngle,angle }
	); 
}

void Transform::Draw(void) {
	if (dynamicFlg) { Attach(); }
	MV1DrawModel(model);
}

void Transform::Release(void) {
	if (model == -1) { return; }
	MV1DeleteModel(model); 
}
