#include <misc/Raycast.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <citro3d.h>

#define INF (CHUNKCACHE_SIZE / 2 * CHUNK_SIZE)

bool Raycast_Cast(World* world, float3 inpos, float3 raydir, Raycast_Result* out) {
	int mapX = FastFloor(inpos.x), mapY = FastFloor(inpos.y), mapZ = FastFloor(inpos.z);

	float xSqr = raydir.x * raydir.x;
	float ySqr = raydir.y * raydir.y;
	float zSqr = raydir.z * raydir.z;

	float deltaDistX = sqrtf(1.f + (ySqr + zSqr) / xSqr);
	float deltaDistY = sqrtf(1.f + (xSqr + zSqr) / ySqr);
	float deltaDistZ = sqrtf(1.f + (xSqr + ySqr) / zSqr);

	int stepX, stepY, stepZ;
	float sideDistX, sideDistY, sideDistZ;
	if (raydir.x < 0) {
		stepX = -1;
		sideDistX = (inpos.x - mapX) * deltaDistX;
	} else {
		stepX = 1;
		sideDistX = (mapX + 1.f - inpos.x) * deltaDistX;
	}
	if (raydir.y < 0) {
		stepY = -1;
		sideDistY = (inpos.y - mapY) * deltaDistY;
	} else {
		stepY = 1;
		sideDistY = (mapY + 1.f - inpos.y) * deltaDistY;
	}
	if (raydir.z < 0) {
		stepZ = -1;
		sideDistZ = (inpos.z - mapZ) * deltaDistZ;
	} else {
		stepZ = 1;
		sideDistZ = (mapZ + 1.f - inpos.z) * deltaDistZ;
	}

	int hit = 0, side = 0, steps = 0;
	while (hit == 0) {
		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDistX;
			mapX += stepX;
			side = 0;
		} else if (sideDistY < sideDistZ) {
			sideDistY += deltaDistY;
			mapY += stepY;
			side = 1;
		} else {
			sideDistZ += deltaDistZ;
			mapZ += stepZ;
			side = 2;
		}
		if (World_GetBlock(world, mapX, mapY, mapZ) != Block_Air) hit = 1;
		// if (world->errFlags & World_ErrUnloadedBlockRequested) break;

		if (steps++ > INF) break;
	}

	switch (side) {
		case 0:  // X Achse
			if (raydir.x > 0.f)
				out->direction = Direction_West;
			else
				out->direction = Direction_East;
			break;
		case 1:  // Y Achse
			if (raydir.y > 0.f)
				out->direction = Direction_Bottom;
			else
				out->direction = Direction_Top;
			break;
		case 2:  // Z Achse
			if (raydir.z > 0.f)
				out->direction = Direction_North;
			else
				out->direction = Direction_South;
			break;
		default:
			printf("Unknown axis! %d\n", side);
			break;
	}

	float3 dist = f3_sub(f3_new(mapX, mapY, mapZ), inpos);
	out->distSqr = f3_magSqr(dist);
	out->x = mapX;
	out->y = mapY;
	out->z = mapZ;

	return hit;
}