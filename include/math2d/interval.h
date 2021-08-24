#ifndef MATH2D_INTERVAL_H
#define MATH2D_INTERVAL_H

#include <math2d\defs.h>

//namespace Math2d {

	class Interval {
	public:

		Scaler min, max;

		// Constructors and setters go here!
		Interval() { }
		Interval(Scaler num_) : min(num_), max(num_) { }
		Interval(Scaler min_, Scaler max_) : min(min_), max(max_) { }
		Interval(const Interval& i) : min(i.min), max(i.max) { }

		void operator+=(Scaler n) {
			min += n;
			max += n;
		}

		bool reversed() const { return (min > max); }

		Scaler dir(const Interval& i) const {
			if (min > i.max) return 1;
			if (max < i.min) return -1;
			return 0;
		}

		bool passN(const Interval& i) const {
			return max < i.min;
		}

		Scaler proj(const Interval& i) const {
			if ( dir(i) == 0 ) {
				Scaler d1 = i.min - max;
				Scaler d2 = i.max - min;
				//Scaler d1 = i.min - max - 2.0f;
				//Scaler d2 = i.max - min + 2.0f;
				//Scaler d1 = i.min - max - 0.005f;
				//Scaler d2 = i.max - min + 0.005f;
				//Scaler d1 = i.min - max + 1 - 0.005f;
				//Scaler d2 = i.max - min - 1 + 0.005f;
				if ( abs(d1) > abs(d2) )
					return d2;
				else
					return d1;
			}
			return 0;
		}

		Scaler projP(const Interval& i) const {
			//if ( dir(i) == 0 )
				return i.min - max;
			//return 0;
		}

		Scaler projN(const Interval& i) const {
			//if ( dir(i) == 0 )
				return i.max - min;
			//return 0;
		}

		bool contain(Scaler n) const {
			if ( n >= min && n <= max )
				return true;
			return false;
		}

		Interval overlap(const Interval& i) const {
			Interval ret(min, max);
			if ( ret.contain(i.min) )
				ret.min = i.min;
			if ( ret.contain(i.max) )
				ret.max = i.max;
			return ret;
		}

		Scaler len() const {
			return max - min;
		}

		void extend(Scaler n) {
			/*if ( min == NaN || max == NaN) {
				min = n;
				max = n;
			}*/
			if ( n < min ) min = n;
			if ( n > max ) max = n;
		}

		void fix() {
			Scaler swap;
			if ( min > max ) {
				swap = min;
				min = max;
				max = swap;
			}
		}
	};

//};


#endif
