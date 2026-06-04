#pragma once

#include "../../pch.h"

#include "../../Utility/Utility.h"

#include "../../Object/Common/Transform.h"
#include "../../Object/Common/DataLoad/ParameterLoad.h"

enum class EFFECT_NAME {

	NON = -1,

	TACKLE_MOVE,

	MAX
};

struct EFFECT_INFO
{
	EFFECT_NAME name = EFFECT_NAME::NON;
	int handle = -1;
	Transform trans{ Vector3() };
	const Transform* follow = nullptr;
	int speed = -1;
};

class EffectBase {
public:

	EffectBase(const ParameterLoad& parameter) : parameter(parameter){}

	virtual ~EffectBase() = default;

	virtual void Update(void)
	{
		if (playHandle == -1) {
			playHandle = PlayEffekseer3DEffect(info.trans.model);
		}
		if (IsEffekseer3DEffectPlaying(playHandle) == -1) {
			end = true;
			return;
		}
		if (info.follow != nullptr) {
			Vector3 pos = info.follow->pos + info.trans.pos.TransMat(MatrixAllMultZXY({ info.follow->angle }));
			Vector3 angle = info.follow->angle + info.trans.angle;
			SetPosPlayingEffekseer3DEffect(playHandle, pos.x, pos.y, pos.z);
			SetRotationPlayingEffekseer3DEffect(playHandle, angle.x, angle.y, angle.z);
		}
		else {
			SetPosPlayingEffekseer3DEffect(playHandle, info.trans.pos.x, info.trans.pos.y, info.trans.pos.z);
			SetRotationPlayingEffekseer3DEffect(playHandle, info.trans.angle.x, info.trans.angle.y, info.trans.angle.z);
		}
		SetScalePlayingEffekseer3DEffect(playHandle, info.trans.scale.x, info.trans.scale.y, info.trans.scale.z);
	}
	virtual void Release(void) 
	{
		DeleteEffekseerEffect(info.trans.model);
	}

	virtual void StopEffect(void)
	{
		StopEffekseer3DEffect(playHandle);
		playHandle = -1;
		Release();
	}

	bool IsEnd(void) { return end; }

	EFFECT_NAME GetName(void) { return info.name; }

protected:

#pragma region パラメーター外部ファイル管理に関する関数
	/// <summary>
	/// パラメーター外部ファイル管理クラスから指定のパラメーターの指定の配列番号の値だけを取得する
	/// </summary>
	/// <param name="parameterName">パラメーターのID</param>
	/// <param name="index">配列番号（指定なしで0）</param>
	/// <returns></returns>
	float GetParameter(const std::string& fileName, const std::string& parameterName, int index = 0)const {
		return parameter.GetParameter(fileName, parameterName, index);
	}

	/// <summary>
	/// パラメーター外部ファイル管理クラスから指定のパラメーターの指定の配列番号の値だけをint型にキャストして取得する
	/// </summary>
	/// <param name="parameterName">パラメーターのID</param>
	/// <param name="index">配列番号（指定なしで0）</param>
	/// <returns></returns>
	int GetParameterToInt(const std::string& fileName, const std::string& parameterName, int index = 0)const {
		return parameter.GetParameterToInt(fileName, parameterName, index);
	}

	/// <summary>
	/// パラメーター外部ファイル管理クラスから指定のパラメーターをVector3構造体にして取得する
	/// </summary>
	/// <param name="parameterName">パラメーターのID</param>
	/// <returns></returns>
	Vector3 GetParameterToVector3(const std::string& fileName, const std::string& parameterName) {
		return parameter.GetParameterToVector3(fileName, parameterName);
	}

	EFFECT_INFO info;

	int playHandle = -1;

	bool end = false;

private:

	const ParameterLoad& parameter;
};