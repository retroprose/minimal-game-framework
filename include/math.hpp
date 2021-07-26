#ifndef MATH_HPP
#define MATH_HPP

//#include <cmath>
//#include <cfloat>

//typedef float Real;     // We are eventually going to turn this into a 32-bit fixed point math object!

//typedef int8_t Int8;
//typedef int16_t Int16;
//typedef int32_t Int32;

//typedef uint8_t UInt8;
//typedef uint16_t UInt16;
//typedef uint32_t UInt32;


//const Real PI = 3.1415927410125732421875f;
//const Real Epsilon = FLT_EPSILON;

//inline Abs(Real x) { return std::abs(x); }
//inline SqRoot(Real x) { return std::sqrt(x); }
//inline Real Sin(Real x) { return std::sin(x); }
//inline Real Cos(Real x) { return std::cos(x); }
//inline Real Atan2(Real y, Real x) { return std::atan2(y, x); }


/*
class Scaler {
public:
    Scaler(int16_t value) { m_value = value; }

    friend Scaler fastAdd(Scaler lhs, Scaler rhs) { return Scaler(lhs.m_value + rhs.m_value); }

//private:
    int16_t m_value;

};
*/



class MersenneTwister {
private:
    // static data and functions
    static const int32_t N = 624;
    static const int32_t M = 397;
    static const uint32_t UPPER_MASK = 0x80000000;
    static const uint32_t LOWER_MASK = 0x7fffffff;
    static const uint32_t MATRIX_A = 0x9908b0df;

    static uint32_t imul(uint32_t a, uint32_t b) {
        uint32_t al = a & 0xffff;
        uint32_t ah = a >> 16;
        uint32_t bl = b & 0xffff;
        uint32_t bh = b >> 16;
        uint32_t ml = al * bl;
        uint32_t mh = ( (((ml >> 16) + al * bh) & 0xffff) + ah * bl ) & 0xffff;
        return (mh << 16) | (ml & 0xffff);
    }

    // local member variables
    int32_t p;
    int32_t q;
    int32_t r;
    uint32_t x[N];

public:
    MersenneTwister() { setSeed(0); }
    MersenneTwister(uint32_t s) { setSeed(s); }

    void setSeed(uint32_t s) {
        x[0] = s;
        for (int32_t i = 1; i < N; i++) {
            x[i] = imul( 1812433253, x[i - 1] ^ (x[i - 1] >> 30) ) + i;
            x[i] &= 0xffffffff;
        }
        p = 0;
        q = 1;
        r = M;
    }

    //double NextDouble() {
    //    return Next(32) / 4294967296.0f;
    //}

    uint32_t next() {
        uint32_t y = (x[p] & UPPER_MASK) | (x[q] & LOWER_MASK);
        x[p] = x[r] ^ (y >> 1) ^ ((y & 1) * MATRIX_A);
        y = x[p];

        if (++p == N) p = 0;
        if (++q == N) q = 0;
        if (++r == N) r = 0;

        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680;
        y ^= (y << 15) & 0xefc60000;
        y ^= (y >> 18);

        return y;
    }

    uint32_t next(uint32_t bits) {
        uint32_t y = (x[p] & UPPER_MASK) | (x[q] & LOWER_MASK);
        x[p] = x[r] ^ (y >> 1) ^ ((y & 1) * MATRIX_A);
        y = x[p];

        if (++p == N) p = 0;
        if (++q == N) q = 0;
        if (++r == N) r = 0;

        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680;
        y ^= (y << 15) & 0xefc60000;
        y ^= (y >> 18);

        return y >> (32 - bits);
    }
};


typedef int32_t Scaler;


class Vector2 {
public:
    Vector2() { }
    Vector2(Scaler X, Scaler Y) : x(X), y(Y) { }

    Scaler x;
    Scaler y;

    Vector2 operator-() const
			{ return Vector2(-x, -y); }

    Vector2 operator+(const Vector2& v) const
        { return Vector2(x+v.x, y+v.y); }

    Vector2 operator-(const Vector2& v)  const
        { return Vector2(x-v.x, y-v.y); }

};






#endif // MATH_HPP
