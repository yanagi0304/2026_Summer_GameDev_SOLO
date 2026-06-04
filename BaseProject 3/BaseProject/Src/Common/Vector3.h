#pragma once

#include "../pch.h"

struct Vector2;

struct Vector3
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

    // コンストラクタ
    Vector3(void);
    Vector3(float x, float y, float z);
    Vector3(float value);
    Vector3(const VECTOR& value);
    Vector3(int x, int y, int z);
    Vector3(int value);

    // 生成
    static Vector3 Xonly(float value);
    static Vector3 Yonly(float value);
    static Vector3 Zonly(float value);
    static Vector3 XYonly(float x, float y);
    static Vector3 XZonly(float x, float z);
    static Vector3 YZonly(float y, float z);

    VECTOR ToVECTOR(void) const;

    // Vector2への変換(X,Y)
    Vector2 ToVector2XY(void)const;
    // Vector2への変換(Y,X)
    Vector2 ToVector2YX(void)const;

    // Vector2への変換(X,Z)
    Vector2 ToVector2XZ(void)const;
    // Vector2への変換(Z,X)
    Vector2 ToVector2ZX(void)const;
    
    // Vector2への変換(Y,Z)
    Vector2 ToVector2YZ(void)const;
    // Vector2への変換(Z,Y)
    Vector2 ToVector2ZY(void)const;

    // 演算
    Vector3 operator+(const Vector3& value) const;
    Vector3 operator+(const VECTOR& value) const;
    Vector3 operator+(float value) const;
    void operator+=(const Vector3& value);
    void operator+=(const VECTOR& value);
    void operator+=(float value);

    Vector3 operator-(const Vector3& value) const;
    Vector3 operator-(const VECTOR& value) const;
    Vector3 operator-(float value) const;
    void operator-=(const Vector3& value);
    void operator-=(const VECTOR& value);
    void operator-=(float value);

    Vector3 operator*(const Vector3& value) const;
    Vector3 operator*(const VECTOR& value) const;
    Vector3 operator*(float value) const;
    void operator*=(const Vector3& value);
    void operator*=(const VECTOR& value);
    void operator*=(float value);

    Vector3 operator/(const Vector3& value) const;
    Vector3 operator/(const VECTOR& value) const;
    Vector3 operator/(float value) const;
    void operator/=(const Vector3& value);
    void operator/=(const VECTOR& value);
    void operator/=(float value);

    float Dot(const Vector3& value) const;

    bool operator==(const Vector3& value) const;
    bool operator==(const VECTOR& value) const;
    bool operator==(float value) const;

    bool operator!=(const Vector3& value) const;
    bool operator!=(const VECTOR& value) const;
    bool operator!=(float value) const;

    bool operator<(const Vector3& value) const;
    bool operator<(const VECTOR& value) const;
    bool operator<(float value) const;

    bool operator<=(const Vector3& value) const;
    bool operator<=(const VECTOR& value) const;
    bool operator<=(float value) const;

    bool operator>(const Vector3& value) const;
    bool operator>(const VECTOR& value) const;
    bool operator>(float value) const;

    bool operator>=(const Vector3& value) const;
    bool operator>=(const VECTOR& value) const;
    bool operator>=(float value) const;

    void operator=(float value);
    void operator=(const Vector3& value);
    void operator=(const VECTOR& value);

    Vector3 operator-() const;

    float LengthSq(void) const;
    float Length(void) const;

    Vector3 Normalized(void) const;
    void Normalize(void);

    Vector3 TransMat(const MATRIX& mat) const;
    void TransMatOwn(const MATRIX& mat);

    Vector3 Abs(void) const;

    float MaxElementF(void) const;
    float MinElementF(void) const;

    Vector3 MaxElement(void) const;
    Vector3 MinElement(void) const;
};