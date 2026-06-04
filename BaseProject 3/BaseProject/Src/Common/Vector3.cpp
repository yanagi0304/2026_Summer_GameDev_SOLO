#include "Vector3.h"

#include <cmath>
#include <algorithm>

#include "Vector2.h"

// コンストラクタ
Vector3::Vector3(void) : x(0), y(0), z(0) {}
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
Vector3::Vector3(float value) : x(value), y(value), z(value) {}
Vector3::Vector3(const VECTOR& value) : x(value.x), y(value.y), z(value.z) {}
Vector3::Vector3(int x, int y, int z) : x((float)x), y((float)y), z((float)z) {}
Vector3::Vector3(int value) : x((float)value), y((float)value), z((float)value) {}

// 生成
Vector3 Vector3::Xonly(float value) { return Vector3(value, 0.0f, 0.0f); }
Vector3 Vector3::Yonly(float value) { return Vector3(0.0f, value, 0.0f); }
Vector3 Vector3::Zonly(float value) { return Vector3(0.0f, 0.0f, value); }
Vector3 Vector3::XYonly(float x, float y) { return Vector3(x, y, 0.0f); }
Vector3 Vector3::XZonly(float x, float z) { return Vector3(x, 0.0f, z); }
Vector3 Vector3::YZonly(float y, float z) { return Vector3(0.0f, y, z); }

// 変換
VECTOR Vector3::ToVECTOR(void) const { return VGet(x, y, z); }

Vector2 Vector3::ToVector2XY(void) const { return Vector2(x, y); }

Vector2 Vector3::ToVector2YX(void) const { return Vector2(y, x); }
Vector2 Vector3::ToVector2XZ(void) const { return Vector2(x, z); }

Vector2 Vector3::ToVector2ZX(void) const { return Vector2(z, x); }

Vector2 Vector3::ToVector2YZ(void) const { return Vector2(y, z); }

Vector2 Vector3::ToVector2ZY(void) const { return Vector2(z, y); }

// 演算
Vector3 Vector3::operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
Vector3 Vector3::operator+(const VECTOR& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
Vector3 Vector3::operator+(float v) const { return Vector3(x + v, y + v, z + v); }
void Vector3::operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; }
void Vector3::operator+=(const VECTOR& v) { x += v.x; y += v.y; z += v.z; }
void Vector3::operator+=(float v) { x += v; y += v; z += v; }

Vector3 Vector3::operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
Vector3 Vector3::operator-(const VECTOR& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
Vector3 Vector3::operator-(float v) const { return Vector3(x - v, y - v, z - v); }
void Vector3::operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; }
void Vector3::operator-=(const VECTOR& v) { x -= v.x; y -= v.y; z -= v.z; }
void Vector3::operator-=(float v) { x -= v; y -= v; z -= v; }

Vector3 Vector3::operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
Vector3 Vector3::operator*(const VECTOR& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
Vector3 Vector3::operator*(float v) const { return Vector3(x * v, y * v, z * v); }
void Vector3::operator*=(const Vector3& v) { x *= v.x; y *= v.y; z *= v.z; }
void Vector3::operator*=(const VECTOR& v) { x *= v.x; y *= v.y; z *= v.z; }
void Vector3::operator*=(float v) { x *= v; y *= v; z *= v; }

Vector3 Vector3::operator/(const Vector3& v) const {
    return Vector3(x / (v.x != 0 ? v.x : 1), y / (v.y != 0 ? v.y : 1), z / (v.z != 0 ? v.z : 1));
}
Vector3 Vector3::operator/(const VECTOR& v) const {
    return Vector3(x / (v.x != 0 ? v.x : 1), y / (v.y != 0 ? v.y : 1), z / (v.z != 0 ? v.z : 1));
}
Vector3 Vector3::operator/(float v) const {
    return Vector3(x / (v != 0 ? v : 1), y / (v != 0 ? v : 1), z / (v != 0 ? v : 1));
}
void Vector3::operator/=(const Vector3& v) {
    x /= (v.x != 0 ? v.x : 1); y /= (v.y != 0 ? v.y : 1); z /= (v.z != 0 ? v.z : 1);
}
void Vector3::operator/=(const VECTOR& v) {
    x /= (v.x != 0 ? v.x : 1); y /= (v.y != 0 ? v.y : 1); z /= (v.z != 0 ? v.z : 1);
}
void Vector3::operator/=(float v) {
    x /= (v != 0 ? v : 1); y /= (v != 0 ? v : 1); z /= (v != 0 ? v : 1);
}

// その他
float Vector3::Dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }

bool Vector3::operator==(const Vector3& v) const { return x == v.x && y == v.y && z == v.z; }
bool Vector3::operator==(const VECTOR& v) const { return x == v.x && y == v.y && z == v.z; }
bool Vector3::operator==(float v) const { return x == v && y == v && z == v; }

bool Vector3::operator!=(const Vector3& v) const { return !(*this == v); }
bool Vector3::operator!=(const VECTOR& v) const { return !(*this == v); }
bool Vector3::operator!=(float v) const { return !(*this == v); }

bool Vector3::operator<(const Vector3& v) const { return x < v.x && y < v.y && z < v.z; }
bool Vector3::operator<(const VECTOR& v) const { return x < v.x && y < v.y && z < v.z; }
bool Vector3::operator<(float v) const { return x < v && y < v && z < v; }

bool Vector3::operator<=(const Vector3& v) const { return x <= v.x && y <= v.y && z <= v.z; }
bool Vector3::operator<=(const VECTOR& v) const { return x <= v.x && y <= v.y && z <= v.z; }
bool Vector3::operator<=(float v) const { return x <= v && y <= v && z <= v; }

bool Vector3::operator>(const Vector3& v) const { return x > v.x && y > v.y && z > v.z; }
bool Vector3::operator>(const VECTOR& v) const { return x > v.x && y > v.y && z > v.z; }
bool Vector3::operator>(float v) const { return x > v && y > v && z > v; }

bool Vector3::operator>=(const Vector3& v) const { return x >= v.x && y >= v.y && z >= v.z; }
bool Vector3::operator>=(const VECTOR& v) const { return x >= v.x && y >= v.y && z >= v.z; }
bool Vector3::operator>=(float v) const { return x >= v && y >= v && z >= v; }

void Vector3::operator=(float v) { x = y = z = v; }
void Vector3::operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z; }
void Vector3::operator=(const VECTOR& v) { x = v.x; y = v.y; z = v.z; }

Vector3 Vector3::operator-() const { return Vector3(-x, -y, -z); }

float Vector3::LengthSq() const { return x * x + y * y + z * z; }
float Vector3::Length() const { return std::sqrt(LengthSq()); }

Vector3 Vector3::Normalized() const {
    float len = Length();
    return (len == 0) ? Vector3() : (*this / len);
}

void Vector3::Normalize() {
    float len = Length();
    if (len != 0) *this /= len;
}

Vector3 Vector3::TransMat(const MATRIX& mat) const {
    if (*this == 0.0f) return Vector3();
    return Vector3(VTransform(ToVECTOR(), mat));
}

void Vector3::TransMatOwn(const MATRIX& mat) {
    if (*this == 0.0f) return;
    *this = VTransform(ToVECTOR(), mat);
}

Vector3 Vector3::Abs() const {
    return Vector3(std::abs(x), std::abs(y), std::abs(z));
}

float Vector3::MaxElementF() const { return (std::max)({ x, y, z }); }
float Vector3::MinElementF() const { return (std::min)({ x, y, z }); }

Vector3 Vector3::MaxElement() const {
    if (x >= y && x >= z) return Vector3::Xonly(x);
    if (y >= z) return Vector3::Yonly(y);
    return Vector3::Zonly(z);
}

Vector3 Vector3::MinElement() const {
    if (x <= y && x <= z) return Vector3::Xonly(x);
    if (y <= z) return Vector3::Yonly(y);
    return Vector3::Zonly(z);
}