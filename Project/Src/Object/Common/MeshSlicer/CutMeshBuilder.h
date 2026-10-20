#pragma once

#include "CutMesh.h"

// MV1モデルから切断用のCutMeshを作る
// （DxLib.h はここで include しない。モデルハンドルはintで受け取る）
class CutMeshBuilder
{
public:

	/// <summary>
	/// MV1モデルの現在の形（アニメーションの姿勢を含む）をCutMeshへ変換する
	/// 頂点はモデル自身のローカル座標（原点がモデルの原点）になる
	/// </summary>
	/// <param name="modelHandle">MV1モデルのハンドル（新しく読み込んだモデルを渡すこと）</param>
	/// <param name="outMesh">（out）変換結果。失敗時は空になる</param>
	/// <param name="frameIndex">変換するフレーム番号（-1 = モデル全体）</param>
	/// <returns>三角形を1枚以上作れたらtrue</returns>
	static bool FromModel(int modelHandle, CutMesh& outMesh, int frameIndex = -1);

private:

	CutMeshBuilder(void) = delete;
};
