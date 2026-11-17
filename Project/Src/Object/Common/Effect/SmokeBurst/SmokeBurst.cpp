#include "SmokeBurst.h"

#include <cmath>
#include <random>

#include "../../Shader/SmokeBurstShader.h"

#include "../../../../Manager/TimeScale/TimeScale.h"

SmokeBurst::SmokeBurst() :
	ActorBase(),
	vertices{},
	indices{},
	elapsed(0.0f),
	emitCount(0)
{
}

void SmokeBurst::Load(void)
{
	// 粒の頂点データを作る
	CreateVertices();

	// 発生するまでは描画しない
	SetIsDraw(false);
}

void SmokeBurst::CreateVertices(void)
{
	// 毎回同じ並びになるよう、乱数の種は固定する
	std::mt19937 engine(20260117u);
	std::uniform_real_distribution<float> rand01(0.0f, 1.0f);

	// 板ポリの4つの角(-1〜1)
	static const float CORNER[VERTEX_PER_PUFF][2] = {
		{ -1.0f, -1.0f },
		{  1.0f, -1.0f },
		{  1.0f,  1.0f },
		{ -1.0f,  1.0f },
	};

	// 中心に残る濃い粒の数
	const int coreNum = 4;

	for (int puff = 0; puff < PUFF_NUM; ++puff) {

		const bool isCore = puff < coreNum;

		// 飛ぶ向き(法線を+Zとした半球の中。cosThetaが小さいほど横へ広がる)
		const float cosTheta = 0.15f + 0.85f * rand01(engine);
		const float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
		const float phi = 6.2831853f * rand01(engine);
		const VECTOR dir = VGet(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

		// 粒ごとの値
		// 中心の粒はあまり飛ばず大きい。周りの粒は速く飛んで少し小さい
		const float speedRate = isCore ? 0.1f + 0.2f * rand01(engine) : 0.5f + 0.5f * rand01(engine);
		const float sizeRate = isCore ? 1.2f + 0.2f * rand01(engine) : 0.7f + 0.5f * rand01(engine);
		const float delay = SmokeBurstShader::MAX_DELAY * rand01(engine);
		const float seedRate = rand01(engine);
		const float riseRate = 0.5f + 0.5f * rand01(engine);
		const float lifeRate = 0.7f + 0.3f * rand01(engine);
		const int brightness = 215 + static_cast<int>(40.0f * rand01(engine));

		for (int corner = 0; corner < VERTEX_PER_PUFF; ++corner) {

			VERTEX3DSHADER& vertex = vertices[puff * VERTEX_PER_PUFF + corner];

			// 位置はシェーダー側で計算するので使わない
			vertex.pos = VGet(0.0f, 0.0f, 0.0f);

			// 飛ぶ向き
			vertex.norm = dir;
			// x:初速の倍率 y:大きさの倍率 z:発生の遅れ
			vertex.tan = VGet(speedRate, sizeRate, delay);
			// x:粒ごとの乱数 y:浮かぶ強さの倍率 z:寿命の倍率
			vertex.binorm = VGet(seedRate, riseRate, lifeRate);

			// 粒ごとの明るさ(Rに入れる)
			vertex.dif = GetColorU8(brightness, brightness, brightness, 255);
			vertex.spc = GetColorU8(0, 0, 0, 0);

			// 板ポリの角
			vertex.u = CORNER[corner][0];
			vertex.v = CORNER[corner][1];
			vertex.su = 0.0f;
			vertex.sv = 0.0f;
		}

		// 板ポリ1枚分のインデックス
		const unsigned short base = static_cast<unsigned short>(puff * VERTEX_PER_PUFF);
		const int index = puff * 6;

		indices[index + 0] = base + 0;
		indices[index + 1] = base + 1;
		indices[index + 2] = base + 2;

		indices[index + 3] = base + 0;
		indices[index + 4] = base + 2;
		indices[index + 5] = base + 3;
	}
}

void SmokeBurst::Emit(const Vector3& pos, const Vector3& normal)
{
	// 黄金比で回して、毎回違う乱数にする
	++emitCount;
	const float seed = std::fmod(emitCount * 0.6180339f, 1.0f);

	// 経過時間を戻して、描画を再開する
	elapsed = 0.0f;
	SetIsDraw(true);

	// シェーダーを差し替える(前の煙が残っていれば破棄される)
	CreateShader(new SmokeBurstShader(pos, normal, seed));
}

void SmokeBurst::SubUpdate(void)
{
	// 描画していない間は何もしない
	if (!GetIsDraw()) { return; }

	// シェーダー側と同じ時間の進め方で、消える時間を判定する
	elapsed += (1.0f / 60.0f) * TimeScale::Get();

	// 最後の粒が消えたら描画を止める
	if (elapsed >= SmokeBurstShader::TOTAL_TIME) { SetIsDraw(false); }
}

void SmokeBurst::SubDraw(void)
{
	// 呼び出し前の合成設定を覚えておく
	int blendMode = DX_BLENDMODE_NOBLEND;
	int blendParam = 0;
	GetDrawBlendMode(&blendMode, &blendParam);

	// 描画設定(深度は見るが書き込まない。両面描画。通常のアルファ合成)
	SetUseLighting(false);
	SetUseZBuffer3D(true);
	SetWriteZBuffer3D(false);
	SetZBufferCmpType(DX_CMP_LESSEQUAL);
	SetUseBackCulling(false);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	// 頂点シェーダーとピクセルシェーダーはActorBase::Drawが適用済み
	DrawPolygonIndexed3DToShader(
		vertices.data(),
		VERTEX_NUM,
		indices.data(),
		POLYGON_NUM
	);

	// 設定を元に戻す
	SetDrawBlendMode(blendMode, blendParam);
	SetUseBackCulling(true);
	SetZBufferCmpType(DX_CMP_LESS);
	SetWriteZBuffer3D(true);
	SetUseLighting(true);
}
