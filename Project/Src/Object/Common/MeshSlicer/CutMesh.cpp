#include "CutMesh.h"

void CutMesh::ToPositionsAndIndices(std::vector<Vector3>& outPositions, std::vector<unsigned int>& outIndices) const
{
	outPositions.clear();
	outIndices.clear();

	outPositions.reserve(vertices.size());
	for (const CutVertex& vertex : vertices) { outPositions.emplace_back(vertex.pos); }

	for (const CutSubMesh& sub : subMeshes) {
		outIndices.insert(outIndices.end(), sub.indices.begin(), sub.indices.end());
	}
}

CutMesh CutMesh::MakeBox(const Vector3& size)
{
	CutMesh mesh;
	CutSubMesh sub(0);

	const float half[3] = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };

	// 面内の4隅（-1 or +1）
	const float cornerU[4] = { -1.0f,  1.0f, 1.0f, -1.0f };
	const float cornerV[4] = { -1.0f, -1.0f, 1.0f,  1.0f };

	// X,Y,Z軸ごとに +面 と -面 を作る
	for (int axis = 0; axis < 3; axis++) {

		const int uAxis = (axis + 1) % 3;
		const int vAxis = (axis + 2) % 3;

		for (int sign = -1; sign <= 1; sign += 2) {

			const unsigned int base = static_cast<unsigned int>(mesh.vertices.size());

			for (int c = 0; c < 4; c++) {
				float p[3] = {};
				float n[3] = {};

				p[axis] = half[axis] * static_cast<float>(sign);
				p[uAxis] = half[uAxis] * cornerU[c];
				p[vAxis] = half[vAxis] * cornerV[c];

				n[axis] = static_cast<float>(sign);

				CutVertex vertex;
				vertex.pos = Vector3(p[0], p[1], p[2]);
				vertex.normal = Vector3(n[0], n[1], n[2]);
				vertex.u = (cornerU[c] + 1.0f) * 0.5f;
				vertex.v = (cornerV[c] + 1.0f) * 0.5f;

				mesh.vertices.push_back(vertex);
			}

			// 巻き順は確認用なので厳密ではない（切断の計算には影響しない）
			if (sign > 0) {
				sub.indices.insert(sub.indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
			}
			else {
				sub.indices.insert(sub.indices.end(), { base, base + 2, base + 1, base, base + 3, base + 2 });
			}
		}
	}

	mesh.subMeshes.push_back(sub);

	return mesh;
}

void CutMesh::DrawWire(unsigned int color, const Vector3& offset) const
{
	for (const CutSubMesh& sub : subMeshes) {
		for (size_t i = 0; i + 2 < sub.indices.size(); i += 3) {

			const Vector3 p0 = vertices[sub.indices[i + 0]].pos + offset;
			const Vector3 p1 = vertices[sub.indices[i + 1]].pos + offset;
			const Vector3 p2 = vertices[sub.indices[i + 2]].pos + offset;

			DrawLine3D(p0.ToVECTOR(), p1.ToVECTOR(), color);
			DrawLine3D(p1.ToVECTOR(), p2.ToVECTOR(), color);
			DrawLine3D(p2.ToVECTOR(), p0.ToVECTOR(), color);
		}
	}
}
