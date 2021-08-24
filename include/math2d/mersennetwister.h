#ifndef MATH2D_MERSENNETWISTER_H
#define MATH2D_MERSENNETWISTER_H

//namespace Math2d {

	class MersenneTwister {
	private:
		// static data and functions
		static const int N = 624;
		static const int M = 397;
		static const unsigned int UPPER_MASK = 0x80000000;
		static const unsigned int LOWER_MASK = 0x7fffffff;
		static const unsigned int MATRIX_A = 0x9908b0df;

		static unsigned int imul(unsigned int a, unsigned int b) {
			unsigned int al = a & 0xffff;
			unsigned int ah = a >> 16;
			unsigned int bl = b & 0xffff;
			unsigned int bh = b >> 16;
			unsigned int ml = al * bl;
			unsigned int mh = ( (((ml >> 16) + al * bh) & 0xffff) + ah * bl ) & 0xffff;
			return (mh << 16) | (ml & 0xffff);
		}

		// local member variables
		int p;
		int q;
		int r;
		unsigned int x[N];

	public:
		MersenneTwister() { SetSeed(0); }
		MersenneTwister(unsigned int s) { SetSeed(s); }

		void SetSeed(unsigned int s) {
			x[0] = s;
			for (int i = 1; i < N; i++) {
				x[i] = imul( 1812433253, x[i - 1] ^ (x[i - 1] >> 30) ) + i;
				x[i] &= 0xffffffff;
			}
			p = 0;
			q = 1;
			r = M;
		}

		double NextDouble() {
			return Next(32) / 4294967296.0f;
		}

		unsigned int Next() {
			unsigned int y = (x[p] & UPPER_MASK) | (x[q] & LOWER_MASK);
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

		unsigned int Next(unsigned int bits) {
			unsigned int y = (x[p] & UPPER_MASK) | (x[q] & LOWER_MASK);
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

//};


#endif
