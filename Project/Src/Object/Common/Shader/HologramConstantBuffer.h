#pragma once

#include "HologramParam.h"

// ホログラム用の定数バッファ(モデル・画像で共用)
class HologramConstantBuffer
{
public:

	HologramConstantBuffer() : handle(-1) {}
	~HologramConstantBuffer() = default;

	// 定数バッファの生成
	void Create(void);

	// 値を書き込んでシェーダーに設定する
	void Set(const HologramParam& param, float time, float textureRate, bool depthOnly);

	// シェーダーへの設定を解除する
	void Reset(void);

	// 定数バッファの解放
	void Release(void);

private:

	// 使用するレジスタ番号
	static constexpr int REGISTER = 4;

	// 定数バッファのハンドル
	int handle;
};
