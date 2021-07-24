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


class Vector2 {
public:
    int32_t x;
    int32_t y;

};






#endif // MATH_HPP
