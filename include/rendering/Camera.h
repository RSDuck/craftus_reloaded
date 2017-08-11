#pragma once

#include <entity/Player.h>

#include <stdbool.h>

#include <citro3d.h>

enum FrustumPlanes {
	Frustum_Near = 0,
	Frustum_Right,
	Frustum_Left,
	Frustum_Top,
	Frustum_Bottom,
	Frustum_Far,

	FrustumPlanes_Count
};

enum FrustumCorners {
	Frustum_NearBottomLeft = 0,
	Frustum_NearBottomRight,
	Frustum_NearTopLeft,
	Frustum_NearTopRight,
	Frustum_FarBottomLeft,
	Frustum_FarBottomRight,
	Frustum_FarTopLeft,
	Frustum_FarTopRight,

	FrustumCorners_Count
};

typedef struct {
	C3D_Mtx projection, view, vp;
	C3D_FVec frustumPlanes[FrustumPlanes_Count];
	float3 frustumCorners[FrustumCorners_Count];

	float near, far, fov;
} Camera;

void Camera_Init(Camera* cam);
void Camera_Update(Camera* cam, Player* player, float iod);

bool Camera_IsPointVisible(Camera* cam, C3D_FVec point);
bool Camera_IsAABBVisible(Camera* cam, C3D_FVec orgin, C3D_FVec size);
