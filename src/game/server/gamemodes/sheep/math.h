#include <base/vmath.h>

class CMath {
public:
	static void Rotate(vec2 Center, vec2 *pPoint, float Rotation) {
		float x = pPoint->x - Center.x;
		float y = pPoint->y - Center.y;
		pPoint->x = (x * cosf(Rotation) - y * sinf(Rotation) + Center.x);
		pPoint->y = (x * sinf(Rotation) + y * cosf(Rotation) + Center.y);	
	}
};