#pragma once
#include <cmath>

#include "Vector3.h"

struct Vector2;

struct Vector2I
{
	int x, y;

#pragma region コンストラクタ
	//Vector2I生成
	Vector2I(void) :x(0), y(0) {}

	//Vector2I生成
	Vector2I(int x,int y):x(x), y(y) {}

	//Vector2I生成
	Vector2I(int value) : x(value), y(value) {}

	//Vector2I生成
	Vector2I(float x, float y) : x((int)x), y((int)y) {}

	//Vector2I生成
	Vector2I(float value) : x((int)value), y((int)value) {}

#pragma endregion

#pragma region Vector2I生成(特殊)

	//Xのみ
	static Vector2I Xonly(int x) { return Vector2I(x, 0); }

	//Yのみ
	static Vector2I Yonly(int y) { return Vector2I(0, y); }

#pragma endregion

	//代入
	void operator=(const int value) { x = value; y = value; }
	void operator=(const Vector2& value);

#pragma region 演算
	//加算
	Vector2I operator+(const Vector2I value) { return Vector2I(x + value.x, y + value.y); }
	void operator+=(const Vector2I value) { x = x + value.x; y = y + value.y; }

};

