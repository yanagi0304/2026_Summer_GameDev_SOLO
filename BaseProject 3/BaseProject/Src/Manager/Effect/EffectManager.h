#pragma once

#include <list>

#include "EffectFactory.h"

#include "../../Object/Common/DataLoad/ParameterLoad.h"

class EffectManager {
public:

	// シングルトン（生成・取得・削除）
	static void CreateIns(void) { if (ins == nullptr) { ins = new EffectManager(); } };
	static EffectManager* GetIns(void) { return ins; };
	static void DeleteIns(void) { if (ins != nullptr) { delete ins; ins = nullptr; } }

	void Update(void);

	void CreateEffect(EFFECT_NAME name, const Transform* follow, const Vector3& local);
	void CreateEffect(EFFECT_NAME name, const Transform& trans);
	void StopEffect(EFFECT_NAME name);
	void StopEffectAll(void);

private:

	EffectManager(void);
	~EffectManager();

	// コピー・ムーブ操作を禁止
	EffectManager(const EffectManager&) = delete;
	EffectManager& operator=(const EffectManager&) = delete;
	EffectManager(EffectManager&&) = delete;
	EffectManager& operator=(EffectManager&&) = delete;

	static EffectManager* ins;

	// パラメーター外部ファイル管理クラス
	ParameterLoad* parameter;

	std::list<EffectBase*> effectInfo;
};