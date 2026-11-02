#pragma once

class Vec2 {
private:
	float x, y;
public:
	Vec2() : x(0), y(0) {}
	Vec2(float value) : x(value), y(value) {}
	Vec2(int value) : x((float)value), y((float)value) {}
	Vec2(float x_, float y_) : x(x_), y(y_) {}
	Vec2(int x_, int y_) : x((float)x_), y((float)y_) {}
	~Vec2() = default;

	float GetX() const;
	float GetY() const;

	void SetX(float value);
	void SetY(float value);
	Vec2 Rotate(float angleRad) const;
	Vec2 RotateDeg(float angleDeg) const;

	float DotProduct(const Vec2& target) const;
	float CrossProduct(const Vec2& target) const;

	float GetAngle() const;

	Vec2 Normalize() const;
	bool IsNormalized() const;

	float Magnitude() const;

	float DistanceTo(const Vec2& target) const;

	Vec2 operator+(const Vec2& value) const;
	Vec2& operator+=(const Vec2& value);

	Vec2 operator-(const Vec2& value) const;
	Vec2& operator-=(const Vec2& value);
	
	Vec2 operator*(int value) const;
	Vec2& operator*=(int value);

	Vec2 operator*(unsigned int value) const;
	Vec2& operator*=(unsigned int value);
	
	Vec2 operator*(float value) const;
	Vec2& operator*=(float value);

	Vec2 operator/(int value) const;	
	Vec2& operator/=(int value);

	Vec2 operator/(unsigned int value) const;
	Vec2& operator/=(unsigned int value);

	Vec2 operator/(float value) const;
	Vec2& operator/=(float value);
};