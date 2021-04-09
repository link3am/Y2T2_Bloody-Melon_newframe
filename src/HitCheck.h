#pragma once
#include "Utilities/BackendHandler.h"
#include <iostream>
class HitCheck
{
public:
	static bool AABB(glm::vec4 a, glm::vec4 b) {
		//left boundary
		float Ap1 = a.x;
		float Bp1 = b.x;
		//right boundary
		float Ap2 = a.x + abs(a.z-0.2f);
		float Bp2 = b.x + abs(b.z-0.2f);
		//bottom boundary
		float Ap3 = a.y;
		float Bp3 = b.y;
		//Up boundary
		float Ap4 = a.y + abs(a.w-0.1f);
		float Bp4 = b.y + abs(b.w-0.1f);
		if (Ap1 <= Bp1 &&
			Bp1 <= Ap2 &&
			Ap3 <= Bp3 &&
			Bp3 <= Ap4) {
			return true;
		}
		else if (Ap1 <= Bp1 &&
			Bp1 <= Ap2 &&
			Ap3 <= Bp4 &&
			Bp4 <= Ap4) {
			return true;
		}
		else if (Ap1 < Bp2 &&
			Bp2 <Ap2 &&
			Ap3 <= Bp3 &&
			Bp3 <= Ap4) {
			return true;
		}
		else if (Ap1 < Bp2 &&
			Bp2 < Ap2 &&
			Ap3 <= Bp4 &&
			Bp4 <= Ap4) {
			return true;
		}
		else {
			return false;
		}
	}
};