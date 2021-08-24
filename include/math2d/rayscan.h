#ifndef MATH2D_RAYSCAN_H
#define MATH2D_RAYSCAN_H

#include <math2d\defs.h>

//namespace Math2d {

	class Ray {
	public:
		void CalcFromVector() {
			end = position + originVector;
			direction = originVector;
			length = direction.Normalize();
		}

		void Calc() {
			originVector = end - position;
			direction = originVector;
			length = direction.Normalize();
		}

		// ray vs shapes here..... need to return point of impact and time of impact
		Scaler Segment(const Vector2 p, Scaler r) {

			Scaler na, nb, dem;

			Vector2 S0, S1, A, B, C;

			// using A as a temp var here, create segment from circle
			A = direction.Right() * r;
			S0 = p + A;
			S1 = p - A;

			A = end - position;
			//A = originVector;
			B = S0 - S1;
			C = position - S0;

			dem = A.y * B.x - A.x * B.y;
			na = B.y * C.x - B.x * C.y;
			nb = A.x * C.y - A.y * C.x;

			//trace("na, nb, dem: " + na + ", " + nb + ", " + dem);

			if (dem == 0) {
				return -1;
			}
			else if (dem > 0) {
				if (na < 0 || na > dem) return -1;
				if (nb < 0 || nb > dem) return -1;
			}
			else {
				if (na > 0 || na < dem) return -1;
				if (nb > 0 || nb < dem) return -1;
			}

			// if we didn't return, then we intersect!
			//trace( "na/dem: " + (na / dem) );
			return (na / dem);
		}

		Vector2 position;
		Vector2 end;
		Vector2 originVector;
		Vector2 direction;
		Scaler length;
	};


	class RayScan {
	public:
		void InitScan(const Vector2& tileSize) {
			X = (int32_t)(ray.position.x / tileSize.x);
			Y = (int32_t)(ray.position.y / tileSize.y);
			done = false;

			max = ray.position - Vector2(X * tileSize.x, Y * tileSize.y);

			if (ray.originVector.x < 0.0f) {
				StepX = -1;
				max.x = (0.0f - max.x) / ray.originVector.x;
				delta.x = -tileSize.x / ray.originVector.x;
			}
			else {
				StepX = 1;
				max.x = (tileSize.x - max.x) / ray.originVector.x;
				delta.x = tileSize.x / ray.originVector.x;
			}

			if (ray.originVector.y < 0.0f) {
				StepY = -1;
				max.y = (0.0f - max.y) / ray.originVector.y;
				delta.y = -tileSize.y / ray.originVector.y;
			}
			else {
				StepY = 1;
				max.y = (tileSize.y - max.y) / ray.originVector.y;
				delta.y = tileSize.y / ray.originVector.y;
			}
		}

		void InitScan(const Ray& scanRay, const Vector2& tileSize) {
			ray = scanRay;
			InitScan(tileSize);
		}

		void Scan() {
			if (max.x > 1.0f && max.y > 1.0f) { done = true; return; }

			if (max.x < max.y) {
				max.x += delta.x;
				X += StepX;
			}
			else {
				max.y += delta.y;
				Y += StepY;
			}
		}

		bool done;

		int32_t X;
		int32_t Y;
		int32_t StepX;
		int32_t StepY;

		Vector2 max;
		Vector2 delta;

		Ray ray;
	};



//};

#endif
