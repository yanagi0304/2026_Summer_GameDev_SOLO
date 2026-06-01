#include"ActorBase.h"

#include"../Application/Application.h"

ActorBase::ActorBase() :
	trans(prevPos),
	collider(),

	dynamicFlg(true),
	isGravity(false),

	pushFlg(true),
	pushWeight(0),

	prevPos(trans.pos),

	accelSum(0.0f),

	isGroundMaster(false),

	isDraw(true),
	isAlphaDraw(false),

	parameter(nullptr)
{
}

ActorBase::ActorBase(const std::string& parameterPath) :
	trans(prevPos),
	collider(),

	dynamicFlg(true),
	isGravity(false),

	pushFlg(true),
	pushWeight(0),

	prevPos(trans.pos),

	accelSum(0.0f, 0.0f, 0.0f),

	isGroundMaster(false),

	isDraw(true),
	isAlphaDraw(false),

	parameter(new ParameterLoad(parameterPath))
{
}

void ActorBase::Init(void)
{
	SubInit();
	
	// パラメータの解放
	if (parameter != nullptr) {
		parameter->Release();
		delete parameter;
		parameter = nullptr;
	}
}

void ActorBase::Update(void)
{
	// 動的オブジェクトは１フレーム前の座標を保持
	if (dynamicFlg) { prevPos = trans.pos; }

	// 派生先追加更新
	SubUpdate();

	// 重力処理
	if (dynamicFlg && isGravity) { Gravity(); }

	// 加速度更新
	if (dynamicFlg) { AccelUpdate(); }

	// 接地判定のリセット
	if (dynamicFlg) { isGroundMaster = false; }
}

void ActorBase::Draw(void)
{
	// 派生先追加描画
	SubDraw();

	// 描画判定
	if (!isDraw) { return; }

	// モデルの描画
	if (!isAlphaDraw) { trans.Draw(); }
}

void ActorBase::AlphaDraw(void)
{
	// 派生先追加アルファ描画
	SubAlphaDraw();

	// 描画判定
	if (!isDraw) { return; }

	// モデルの描画（アルファ描画）
	if (isAlphaDraw) { trans.Draw(); }

	// 当たり判定のデバッグ描画
	if (App::GetIns().IsDrawDebug()) {
		for (ColliderBase*& c : collider) { if (c->GetJudge()) c->DrawDebug(); }
	}
}

void ActorBase::Release(void)
{
	// 派生先追加解放
	SubRelease();

	// パラメータの解放
	if (parameter != nullptr) {
		parameter->Release();
		delete parameter;
		parameter = nullptr;
	}

	// 当たり判定情報を解放
	for (ColliderBase*& c : collider) {
		if (!c) { continue; }
		delete c;
		c = nullptr;
	}
	collider.clear();

	// モデル制御情報の解放
	trans.Release();
}

void ActorBase::AccelUpdate(void)
{
	// 横軸(横軸は減衰もする)～～～～～～～～～～～～～～～～～～～～～～～～～～～～～

	// 横軸のみの加速度の長さを求める
	float widthAccelLen = sqrt(accelSum.x * accelSum.x + accelSum.z * accelSum.z);

	// 横軸の加速度がある場合、減衰処理を行う
	if (widthAccelLen > 0.0f) {

		// 横軸の加速度が減衰の半分以下の場合は、横軸の加速度を0にする
		if (widthAccelLen <= ATTENUATION * 0.5f) { accelSum.x = accelSum.z = 0.0f; }

		float scale = (widthAccelLen - ATTENUATION) / widthAccelLen;
		if (widthAccelLen > ACCEL_MAX) { scale = ACCEL_MAX / widthAccelLen; }

		accelSum.x *= scale;
		accelSum.z *= scale;
	}

	// ～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～


	// 加速度を座標に反映
	if (accelSum != 0.0f) { trans.pos += accelSum; }
}

void ActorBase::Gravity(void)
{
	accelSum.y += GRAVITY;
	if (accelSum.y < GRAVITY_MAX) { accelSum.y = GRAVITY_MAX; }
}
