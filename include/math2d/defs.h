#ifndef MATH2D_DEFS_H
#define MATH2D_DEFS_H

#include <cmath>
#include <cfloat>

//namespace Math2d {

	typedef float Scaler;
	typedef int Integer;

	const Scaler PI = 3.1415927410125732421875f;
	const Scaler Epsilon = FLT_EPSILON;

	template<class T>
	T Abs(T t) { return std::abs(t); }

	template<class T>
	T Min(T a, T b) { return std::min(a, b); }

	template<class T>
	T Max(T a, T b) { return std::max(a, b); }

	template<class T>
	T Floor(T t) { return std::floor(t); }

	template<class T>
	T Clamp(T value, T low, T high) { return Max(low, Min(value, high)); }

	template<class T>
	T Cos(T t) { return cos(t); }

	template<class T>
	T Sin(T t) { return sin(t); }

	template<class T>
	T Atan2(T y, T x) { return std::atan2(y, x); }

	template<class T>
	T SqRoot(T t) { return std::sqrt(t); }

//};


#endif

