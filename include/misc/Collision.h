#pragma once

#include <stdbool.h>

#include <misc/VecMath.h>

typedef struct { float3 min, max; } Box;

inline Box Box_Create(float x, float y, float z, float w, float h, float d) { return (Box){f3_new(x, y, z), f3_new(x + w, y + h, z + d)}; }
inline bool Box_Contains(Box box, float x, float y, float z) {
	return box.min.x <= x && box.min.y <= y && box.min.z <= z && box.max.x > x && box.max.y > y && box.max.z > z;
}

bool Collision_BoxIntersect(Box a, Box b, int ignore_faces, float3* ncoll,  // normal of collision.
			    float* dcoll,				    // depth of intersection.
			    int* fcoll);				    // face intersected.
