#pragma once

#include "../../../Common/Vector3.h"

struct Transform
{
	// モデル
	int model;

	// 座標
	Vector3 pos;
	// 1フレーム前の座標
	Vector3 prevPos;

	// 描画する際の座標のズレを補完する用の変数。モデルを中心を軸に描画するため
	Vector3 centerDiff;

	// 角度
	Vector3 angle;
	// 描画する際の角度のズレを補完する用の変数。
	Vector3 localAngle;

	// スケール
	Vector3 scale;

	// 動的オブジェクトかどうか（true = 動的オブジェクト、false = 静的オブジェクト）
	bool dynamicFlg;

	/// 生成
	Transform(void) :
		model(-1),

		pos(),
		prevPos(),

		centerDiff(),

		angle(),
		localAngle(),

		scale(1),

		dynamicFlg(true)
	{
	}

	// 現在の移動量
	Vector3 Velocity(void)const;

	// 角度を行列化したもの
	MATRIX AngleMat(void)const;

	// 渡されたVector3構造体を自身の角度情報で回転させたVector3構造体を返す
	Vector3 VTrans(const Vector3& v)const;
	// 渡されたVECTOR構造体を自身の角度情報で回転させたVector3構造体を返す
	Vector3 VTrans(const VECTOR& v)const;

	// 角度を代入
	void SetAngleDeg(const Vector3& deg);
	void SetAngleXDeg(float deg);
	void SetAngleYDeg(float deg);
	void SetAngleZDeg(float deg);

	// 角度に加算
	void AddAngleDeg(const Vector3& deg);
	void AddAngleXDeg(float deg);
	void AddAngleYDeg(float deg);
	void AddAngleZDeg(float deg);

	/// <summary>
	/// モデルをロード
	/// </summary>
	/// <param name="path">モデルのパス（Data/Model/～～.mv1）</param>
	void LoadModel(std::string path);

	/// <summary>
	/// モデルを複製
	/// </summary>
	/// <param name="model">モデルのハンドル</param>
	void Duplicate(int model);

	/// <summary>
	/// エフェクトをロード
	/// </summary>
	/// <param name="path">モデルのパス（Data/Effect/～～.efk）</param>
	void LoadEffect(std::string path);

	// 制御情報をモデルに適用
	void Attach(void);

	// モデルを描画（変数情報をモデルに適用してから）
	void Draw(void);

	// モデルを解放
	void Release(void);
};