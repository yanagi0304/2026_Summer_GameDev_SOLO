#pragma once
#include <Eigen/Core>
#include <igl/slice.h>
#include <vector>
#include "../../../Common/Vector3.h"


class MeshSlicer
{
public:

	static constexpr float DefaultOffset = 0.0f;

	//モデルを切断して、片側の新しいモデルハンドルを返す
	int SliceModel(int baseModel, Vector3 planeNormal, Vector3 planePoint)
	{
		int vertexCount = MV1GetMeshVertexNum(baseModel, 0);
		int indexCount = MV1GetMeshTriangleNum(baseModel, 0);

		std::vector<VERTEX_3D> dxlVertices(vertexCount);

	}

};

