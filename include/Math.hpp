#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>
#include <cfloat>

typedef float Real;     // We are eventually going to turn this into a 32-bit fixed point math object!

typedef int8_t Int8;
typedef int16_t Int16;
typedef int32_t Int32;

typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;


const Real PI = 3.1415927410125732421875f;
const Real Epsilon = FLT_EPSILON;

//inline Abs(Real x) { return std::abs(x); }
inline SqRoot(Real x) { return std::sqrt(x); }
inline Real Sin(Real x) { return std::sin(x); }
inline Real Cos(Real x) { return std::cos(x); }
inline Real Atan2(Real y, Real x) { return std::atan2(y, x); }


// simple 2d vector class
class Vector2 {
public:
    Real x;
    Real y;

    Vector2() { }
    Vector2(Real x_, Real y_) : x(x_), y(y_) { }

    // comparison operators
    bool operator==(const Vector2& v) const
        { return (x == v.x && y == v.y); }

    bool operator!=(const Vector2& v)  const
        { return (x != v.x || y != v.y); }

    // friend functions
    friend Vector2 operator*(const Vector2& v, Real s) { return Vector2(v.x * s, v.y * s); }
    friend Vector2 operator/(const Vector2& v, Real s) { return Vector2(v.x / s, v.y / s); }
    friend Vector2 operator*(Real s, const Vector2& v) { return Vector2(s * v.x, s * v.y); }
    friend Vector2 operator/(Real s, const Vector2& v) { return Vector2(s / v.x, s / v.y); }

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

    Vector2& operator*=(Real s) {
        x *= s;
        y *= s;
        return *this;
    }

    Vector2& operator/=(Real s) {
        Real inv = 1.0f / s;
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
    Real Dot(const Vector2& v) const
        { return x * v.x + y * v.y; }

    Real LeftDot(const Vector2& v) const
        { return y * v.x - x * v.y; }

    Real RightDot(const Vector2& v) const
        { return x * v.y - y * v.x; }

    // vector length
    Real Length() const
        { return SqRoot(x*x + y*y); }

    Real LengthSquared() const
        { return x*x + y*y; }

    // vector normlaization
    Vector2 NormalizedCopy() const {
        Real length = SqRoot(x*x + y*y);
        if (length < Epsilon) {
            return Vector2(*this);
        }
        Real invLength = 1.0f / length;
        Vector2 v(*this);
        v.x *= invLength;
        v.y *= invLength;
        return v;
    }

    Vector2 NormalizedCopy(Real length) const {
        if (length < Epsilon) {
            return Vector2(*this);
        }
        Real invLength = 1.0f / length;
        Vector2 v(*this);
        v.x *= invLength;
        v.y *= invLength;
        return v;
    }

    Real Normalize() {
        Real length = SqRoot(x*x + y*y);
        if (length < Epsilon) {
            return 0.0f;
        }
        Real invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;
        return length;
    }

    Real Normalize(Real length) {
        if (length < Epsilon) {
            return 0.0f;
        }
        Real invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;
        return length;
    }

    Vector2 MidPoint(const Vector2& v) const
        { return Vector2( (x+v.x) * 0.5f, (y+v.y) * 0.5f ); }

    // angle with respect to unit x axis
    Real Angle() const
        { return Atan2(y, x); }

    // angle with respect to abritraty second vector
    Real Angle(const Vector2& v) const
        { return Atan2(y * v.x - x * v.y, x * v.x + y * v.y); }

    // rotate vector by angle
    Vector2 Rotate(Real a) const {
        Real c = Cos(a);
        Real s = Sin(a);
        return Vector2(x * c - y * s, x * s + y * c);
    }

    Vector2 Reflect(const Vector2& n) const {
        // -2 * (V dot N)*N + V
        return Vector2(-2.0f * Dot(n) * n + *this);
    }

};

#endif // MATH_HPP
