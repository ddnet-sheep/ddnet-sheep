#include <base/vmath.h>

//#include <algorithm>
//#include <iterator>
#include <random>

class CMath {
public:
	static void Rotate(vec2 Center, vec2 *pPoint, float Rotation) {
		float x = pPoint->x - Center.x;
		float y = pPoint->y - Center.y;
		pPoint->x = (x * cosf(Rotation) - y * sinf(Rotation) + Center.x);
		pPoint->y = (x * sinf(Rotation) + y * cosf(Rotation) + Center.y);	
	}

	static int RandGeometric(std::mt19937 &rng, int Min, int Max, double p)
	{
		if(Max < Min)
			std::swap(Min, Max);
		p = std::clamp(p, 1e-9, 1.0 - 1e-9);
		std::geometric_distribution<int> geo(p); 
		int range = Max - Min;
		int k = geo(rng);
		if(k > range)
			k = range; 
		return Min + k;
	}

	static float GetAngle(vec2 Dir) {
		if (Dir.x == 0 && Dir.y == 0)
			return 0.0f;

		float a = atanf(Dir.y / Dir.x);
		if (Dir.x < 0)
			a = a + pi;

		return a;
	}

	static vec2 GetDir(float Angle) {
		return vec2(cosf(Angle), sinf(Angle));
	}
};