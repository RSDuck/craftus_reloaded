#pragma once

#include <world/World.h>

#include <misc/VecMath.h>

typedef struct {
	int x, y, z;
	float distSqr;
	Direction direction;
} Raycast_Result;

bool Raycast_Cast(World* world, float3 inpos, float3 raydir, Raycast_Result* out);