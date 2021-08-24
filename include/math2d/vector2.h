#ifndef MATH2D_VECTOR2_H
#define MATH2D_VECTOR2_H

#include <math2d\defs.h>

//namespace Math2d {

	// simple 2d vector class
	class Vector2 {
	public:

		Scaler x;
		Scaler y;

		Vector2() { }
		Vector2(Scaler x_, Scaler y_) : x(x_), y(y_) { }

		// comparison operators
		bool operator==(const Vector2& v) const
			{ return (x == v.x && y == v.y); }

		bool operator!=(const Vector2& v)  const
			{ return (x != v.x || y != v.y); }

		// friend functions
		friend Vector2 operator*(const Vector2& v, Scaler s) { return Vector2(v.x * s, v.y * s); }
		friend Vector2 operator/(const Vector2& v, Scaler s) { return Vector2(v.x / s, v.y / s); }
		friend Vector2 operator*(Scaler s, const Vector2& v) { return Vector2(s * v.x, s * v.y); }
		friend Vector2 operator/(Scaler s, const Vector2& v) { return Vector2(s / v.x, s / v.y); }

		// operations
		Vector2 operator-() const
			{ return Vector2(-x, -y); }

		Vector2 operator+(const Vector2& v) const
			{ return Vector2(x+v.x, y+v.y); }

		Vector2 operator-(const Vector2& v)  const
			{ return Vector2(x-v.x, y-v.y); }

		Vector2& operator+=(const Vector2& v) {
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2& operator-=(const Vector2& v) {
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2& operator*=(Scaler s) {
			x *= s;
			y *= s;
			return *this;
		}

		Vector2& operator/=(Scaler s) {
			Scaler inv = 1.0f / s;
			x *= inv;
			y *= inv;
			return *this;
		}

		// left and right hand perpendicular vectors
		Vector2 Left() const
			{ return Vector2(-y, x); }

		Vector2 Right() const
			{ return Vector2(y, -x); }

		// Dot products
		Scaler Dot(const Vector2& v) const
			{ return x * v.x + y * v.y; }

		Scaler LeftDot(const Vector2& v) const
			{ return y * v.x - x * v.y; }

		Scaler RightDot(const Vector2& v) const
			{ return x * v.y - y * v.x; }

		// vector length
		Scaler Length() const
			{ return SqRoot(x*x + y*y); }

		Scaler LengthSquared() const
			{ return x*x + y*y; }

		// vector normlaization
		Vector2 NormalizedCopy() const {
			Scaler length = SqRoot(x*x + y*y);
			if (length < Epsilon) {
				return Vector2(*this);
			}
			Scaler invLength = 1.0f / length;
			Vector2 v(*this);
			v.x *= invLength;
			v.y *= invLength;
			return v;
		}

		Vector2 NormalizedCopy(Scaler length) const {
			if (length < Epsilon) {
				return Vector2(*this);
			}
			Scaler invLength = 1.0f / length;
			Vector2 v(*this);
			v.x *= invLength;
			v.y *= invLength;
			return v;
		}

		Scaler Normalize() {
			Scaler length = SqRoot(x*x + y*y);
			if (length < Epsilon) {
				return 0.0f;
			}
			Scaler invLength = 1.0f / length;
			x *= invLength;
			y *= invLength;
			return length;
		}

		Scaler Normalize(Scaler length) {
			if (length < Epsilon) {
				return 0.0f;
			}
			Scaler invLength = 1.0f / length;
			x *= invLength;
			y *= invLength;
			return length;
		}

		Vector2 MidPoint(const Vector2& v) const
			{ return Vector2( (x+v.x) * 0.5f, (y+v.y) * 0.5f ); }

		// angle with respect to unit x axis
		Scaler Angle() const
			{ return Atan2(y, x); }

		// angle with respect to abritraty second vector
		Scaler Angle(const Vector2& v) const
			{ return Atan2(y * v.x - x * v.y, x * v.x + y * v.y); }

		// rotate vector by angle
		Vector2 Rotate(Scaler a) const {
			Scaler c = Cos(a);
			Scaler s = Sin(a);
			return Vector2(x * c - y * s, x * s + y * c);
		}

		Vector2 Reflect(const Vector2& n) const {
			// -2 * (V dot N)*N + V
			return Vector2(-2.0f * Dot(n) * n + *this);
		}

	};



//};

#endif
