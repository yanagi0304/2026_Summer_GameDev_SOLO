#pragma once
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

#include <DxLib.h>
#include "../Common/Vector2.h"
#include "../Common/Vector3.h"

//ラジアン・度変換用
static constexpr float RAD2DEG = (180.0f / DX_PI_F);
static constexpr float DEG2RAD = (DX_PI_F / 180.0f);

//極小値
static constexpr float kEpsilonNormalSqrt = 1e-15f;

//四捨五入
static int Round(float v) { return static_cast<int>(roundf(v)); }


//文字列分割
static std::vector<std::string> Split(const std::string& line, char delimiter)
{
	std::istringstream stream(line);
	std::string field;
	std::vector<std::string> result;

	while (std::getline(stream, field, delimiter))
	{
		result.push_back(field);
	}

	return result;
}

//ラジアン(rad)から度(deg)
static double Rad2Deg(double rad) { return rad * static_cast<double>(RAD2DEG); }
static float Rad2Deg(float rad) { return rad * RAD2DEG; }
static int Rad2Deg(int rad) { return Round(static_cast<float>(rad) * RAD2DEG); }

//度(deg)からラジアン(rad)
static double Deg2Rad(double deg) { return deg * static_cast<double>(DEG2RAD); }
static float Deg2Rad(float deg) { return deg * DEG2RAD; }
static int Deg2Rad(int deg) { return Round(static_cast<float>(deg) * DEG2RAD); }

//一括変換
static VECTOR Deg2Rad(const VECTOR& deg) { return { Deg2Rad(deg.x), Deg2Rad(deg.y), Deg2Rad(deg.z) }; }
static Vector3 Deg2Rad(const Vector3& deg) { return { Deg2Rad(deg.x), Deg2Rad(deg.y), Deg2Rad(deg.z) }; }

//0～360の範囲に収める
static double DegIn360(double deg) {
	deg = fmod(deg, 360.0);
	if (deg < 0.0f)
	{
		deg += 360.0;
	}
	return deg;
}

//0(0)～2π(360度)の範囲に収める
static double RadIn2PI(double rad) {
	rad = fmod(rad, DX_TWO_PI);
	if (rad < 0.0)
	{
		rad += DX_TWO_PI;
	}
	return rad;
}

// 回転が少ない方の回転向きを取得する(時計回り:1、反時計回り:-1)
static int DirNearAroundRad(float from, float to) {
	float ret = 1.0f;

	float diff = to - from;

	if (diff >= 0.0f) {
		// 比較元よりも時計回りに位置する

		// でも、180度以上離れているので、反時計回りの方が近い
		if (diff > DX_PI_F) { ret = -1.0f; }
		// 時計回り
		else { ret = 1.0f; }

	}
	else {
		// 比較元よりも反時計回りに位置する

		// でも、180度以上離れているので、時計回りの方が近い
		if (diff < -DX_PI_F) { ret = 1.0f; }
		// 反時計回り
		else { ret = -1.0f; }
	}

	return static_cast<int>(ret);
}

// 回転が少ない方の回転向きを取得する(時計回り:1、反時計回り:-1)
static int DirNearAroundDeg(float from, float to) {
	float ret = 1.0f;

	float diff = to - from;

	if (diff >= 0.0f) {
		// 比較元よりも時計回りに位置する

		// でも、180度以上離れているので、反時計回りの方が近い
		if (diff > 180.0f) { ret = -1.0f; }
		// 時計回り
		else { ret = 1.0f; }
	}
	else {
		// 比較元よりも反時計回りに位置する

		// でも、180度以上離れているので、時計回りの方が近い
		if (diff < -180.0f) { ret = 1.0f; }
		// 反時計回り
		else { ret = -1.0f; }
	}

	return static_cast<int>(ret);
}

//線形補間
static int Lerp(int start, int end, float t) {
	if (t >= 1.0f) { return end; }

	int ret = start;
	ret += Round(t * static_cast<float>(end - start));
	return ret;
}
static float Lerp(float start, float end, float t) {
	if (t >= 1.0f) { return end; }

	float ret = start;
	ret += t * (end - start);
	return ret;
}
static double Lerp(double start, double end, double t) {
	if (t >= 1.0) { return end; }

	double ret = start;
	ret += t * (end - start);
	return ret;
}
static Vector2 Lerp(const Vector2& start, const Vector2& end, float t) {
	if (t >= 1.0f) { return end; }

	Vector2 ret = start;
	ret.x += Round(t * static_cast<float>((end.x - start.x)));
	ret.y += Round(t * static_cast<float>((end.y - start.y)));
	return ret;
}

//線形補間(角度)
static double LerpDeg(double start, double end, double t) {
	double ret;
	
	double diff = end - start;
	if (diff < -180.0) {
		end += 360.0;
		ret = Lerp(start, end, t);
		if (ret >= 360.0) { ret -= 360.0; }
	}
	else if (diff > 180.0) {
		end -= 360.0;
		ret = Lerp(start, end, t);
		if (ret < 0.0) { ret += 360.0; }
	}
	else { ret = Lerp(start, end, t); }

	return ret;

}

//ベジェ曲線
static Vector2 Bezier(const Vector2& p1, const Vector2& p2, const Vector2& p3, float t) {
	Vector2 a = Lerp(p1, p2, t);
	Vector2 b = Lerp(p2, p3, t);
	return Lerp(a, b, t);
}

//符号
static float Signed(float x) { return (x < 0) ? -1.0f : 1.0f; }

//画像読み込み
static void LoadImg(int& handle, std::string path) {
	handle = LoadGraph(path.c_str());
	if(handle == -1) {
		printfDx("画像の読み込みに失敗しました");
	}
}

//画像読み込み
static int LoadImg(std::string path) {
	int ret = 0;

	ret = LoadGraph(path.c_str());
	if (ret == -1) {
		printfDx("画像の読み込みに失敗しました");
	}

	return ret;
}

// スプライト画像読み込み
static void LoadArrayImg(std::string path, int AllNum, int XNum, int YNum, int XSize, int YSize, int* handleArray) {
	int err = 0;

	err = LoadDivGraph(path.c_str(), AllNum, XNum, YNum, XSize, YSize, handleArray);

	if (err == -1) {
		printfDx("画像読み込みに失敗しました");
	}
}

static void LoadArrayImg(std::string path, int AllNum, int XNum, int YNum, int XSize, int YSize, std::vector<int>& handleArray) {
	handleArray.resize(AllNum);

	int err = LoadDivGraph(path.c_str(), AllNum, XNum, YNum, XSize, YSize, handleArray.data());

	if (err == -1) {
		printfDx("画像読み込みに失敗しました: %s\n", path.c_str());
		return;
	}
}

#pragma region 各順番の（VECTOR or Vector3）構造体を行列に変換する関数（引数にいくつでも入れられる仕様 左から順に計算される）

template <typename T>
static MATRIX MatrixAllMultX(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) { m = MMult(m, MGetRotX(vec.x)); }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultY(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) { m = MMult(m, MGetRotY(vec.y)); }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultZ(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) { m = MMult(m, MGetRotZ(vec.z)); }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultXY(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotX(vec.x));
        m = MMult(m, MGetRotY(vec.y));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultXZ(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotX(vec.x));
        m = MMult(m, MGetRotZ(vec.z));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultXYZ(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotX(vec.x));
        m = MMult(m, MGetRotY(vec.y));
        m = MMult(m, MGetRotZ(vec.z));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultXZY(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotX(vec.x));
        m = MMult(m, MGetRotZ(vec.z));
        m = MMult(m, MGetRotY(vec.y));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultYX(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotY(vec.y));
        m = MMult(m, MGetRotX(vec.x));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultYZ(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotY(vec.y));
        m = MMult(m, MGetRotZ(vec.z));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultYXZ(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotY(vec.y));
        m = MMult(m, MGetRotX(vec.x));
        m = MMult(m, MGetRotZ(vec.z));
    }
    return m;
}


template <typename T>
static MATRIX MatrixAllMultYZX(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotY(vec.y));
        m = MMult(m, MGetRotZ(vec.z));
        m = MMult(m, MGetRotX(vec.x));
    }
    return m;
}


template <typename T>
static MATRIX MatrixAllMultZX(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotZ(vec.z));
        m = MMult(m, MGetRotX(vec.x));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultZY(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotZ(vec.z));
        m = MMult(m, MGetRotY(vec.y));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultZXY(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotZ(vec.z));
        m = MMult(m, MGetRotX(vec.x));
        m = MMult(m, MGetRotY(vec.y));
    }
    return m;
}

template <typename T>
static MATRIX MatrixAllMultZYX(const std::initializer_list<T>& vecs) {
    MATRIX m = MGetIdent();
    for (const auto& vec : vecs) {
        m = MMult(m, MGetRotZ(vec.z));
        m = MMult(m, MGetRotY(vec.y));
        m = MMult(m, MGetRotX(vec.x));
    }
    return m;
}

#pragma endregion

//行列を使ったモデルの座標と向きのセット

template <typename T>
static void MV1ModelMatrix(int& model, const T& pos, const std::initializer_list<T>& angle) {
	MATRIX m = MGetIdent();
	//角度セットを順に合成
	for (const auto& a : angle) {
		m = MMult(m, MGetRotX(a.x));
		m = MMult(m, MGetRotY(a.y));
		m = MMult(m, MGetRotZ(a.z));
	}

	MV1SetRotationMatrix(model, m);
	MV1SetPosition(model, pos);
}