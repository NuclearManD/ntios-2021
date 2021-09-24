
#ifndef NTIOS_MATH_H
#define NTIOS_MATH_H

#include <cmath>

class Vector3 {
public:
	double x, y, z;

	Vector3() : x(0), y(0), z(0) {};
	Vector3(double x, double y, double z) : x(x), y(y), z(z) {};

	double magnitude() { return sqrt(x*x + y*y + z*z); };

	Vector3 operator+(const Vector3& b) { return Vector3(x + b.x, y + b.y, z + b.z); }
	Vector3 operator-(const Vector3& b) { return Vector3(x - b.x, y - b.y, z - b.z); }
	Vector3 operator*(double b) { return Vector3(x*b, y*b, z*b); }
	Vector3 operator/(double b) { return Vector3(x/b, y/b, z/b); }
};

class Vector2 {
public:
	double x, y;

	Vector2() : x(0), y(0) {};
	Vector2(double x, double y) : x(x), y(y) {};

	double magnitude() { return sqrt(x*x + y*y); };

	Vector2 operator+(const Vector2& b) { return Vector2(x + b.x, y + b.y); };
	Vector2 operator-(const Vector2& b) { return Vector2(x - b.x, y - b.y); };
	Vector2 operator*(double b) { return Vector2(x*b, y*b); };
	Vector2 operator/(double b) { return Vector2(x/b, y/b); };
};

class Quaternion {
public:
	double w, x, y, z;

	Quaternion() : w(0), x(0), y(0), z(0) {};
	Quaternion(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {};

	static double* H_product(double* q, double* r, double* dest);
	Vector3 rotateVector(Vector3& vector);

};

#endif
