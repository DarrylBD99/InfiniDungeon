#include "Vector.hpp"
#include <numbers>
#include <cmath>

Vec2 round(const Vec2& value)
{
	return Vec2{
		std::round(value.GetX()),
		std::round(value.GetY())
	};
}

float Vec2::GetX() const {
	return x;
}

float Vec2::GetY() const {
	return y;
}

void Vec2::SetX(float value) {
	x = value;
}

void Vec2::SetY(float value) {
	y = value;
}

Vec2 Vec2::Rotate(float angleRad) const {
	float x_ = x * std::cosf(angleRad) - y * std::sinf(angleRad);
	float y_ = x * std::sinf(angleRad) + y * std::cosf(angleRad);

	return { x_, y_ };
}

Vec2 Vec2::RotateDeg(float angleDeg) const {
	return Rotate(angleDeg * ((float)std::numbers::pi / 180.0f));
}

float Vec2::GetAngle() const {
	return atan2(y, x);
}

void Vec2::abs() {
	x = std::abs(x);
	y = std::abs(y);
}

float Vec2::DotProduct(const Vec2& value) const {
	return x * value.x + y * value.y;
}

float Vec2::CrossProduct(const Vec2& value) const {
	return x * value.y - y * value.x;
}

Vec2 Vec2::Normalize() const {
	float length = Magnitude();
	if (length == 0.0f) return { 0, 0 };
	return *this / length;
}

bool Vec2::IsNormalized() const {
	float length = Magnitude();
	return std::abs(length - 1.0f) < 0.0001f;
}

float Vec2::Magnitude() const {
	return sqrtf( (x * x) + (y * y) );
}

float Vec2::DistanceTo(const Vec2& target) const {
	Vec2 temp = target - *this;
	return temp.Magnitude();
}

Vec2 Vec2::operator+(const Vec2& value) const {
	return { x + value.x, y + value.y };
}

Vec2& Vec2::operator+=(const Vec2& value) {
	x += value.x;
	y += value.y;

	return *this;
}

Vec2 Vec2::operator-(const Vec2& value) const {
	return { x - value.x, y - value.y };
}

Vec2& Vec2::operator-=(const Vec2& value) {
	x -= value.x;
	y -= value.y;

	return *this;
}

Vec2 Vec2::operator*(int value) const {
	return { x * value, y * value };
}

Vec2& Vec2::operator*=(int value) {
	x *= value;
	y *= value;

	return *this;
}

Vec2 Vec2::operator*(unsigned int value) const {
	return { x * value, y * value };
}

Vec2& Vec2::operator*=(unsigned int value) {
	x *= value;
	y *= value;

	return *this;
}

Vec2 Vec2::operator*(float value) const {
	return { x * value, y * value };
}

Vec2& Vec2::operator*=(float value) {
	x *= value;
	y *= value;

	return *this;
}

Vec2 Vec2::operator/(int value) const {
	return { x / value, y / value };
}

Vec2& Vec2::operator/=(int value) {
	x /= value;
	y /= value;

	return *this;
}

Vec2 Vec2::operator/(unsigned int value) const {
	return { x / value, y / value };
}

Vec2& Vec2::operator/=(unsigned int value) {
	x /= value;
	y /= value;

	return *this;
}

Vec2 Vec2::operator/(float value) const {
	return { x / value, y / value };
}

Vec2& Vec2::operator/=(float value) {
	x /= value;
	y /= value;

	return *this;
}

bool Vec2::operator==(Vec2 value) const {
	return (value.x == x && value.y == y);
}