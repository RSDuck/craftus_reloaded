#pragma once

#include <math.h>

#include <misc/NumberUtils.h>

typedef union {
	float v[3];
	struct {
		float x, y, z;
	};
} float3;

inline float3 f3_new(float x, float y, float z) { return (float3){{x, y, z}}; }

inline float3 f3_add(float3 a, float3 b) { return f3_new(a.x + b.x, a.y + b.y, a.z + b.z); }
inline float3 f3_sub(float3 a, float3 b) { return f3_new(a.x - b.x, a.y - b.y, a.z - b.z); }
inline float3 f3_scl(float3 a, float b) { return f3_new(a.x * b, a.y * b, a.z * b); }

inline float f3_dot(float3 a, float3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float3 f3_crs(float3 a, float3 b) { return f3_new(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

inline float f3_mag(float3 vec) { return sqrtf(f3_dot(vec, vec)); }
inline float f3_magSqr(float3 vec) { return f3_dot(vec, vec); }
inline float3 f3_nrm(float3 vec) {
	float m = f3_mag(vec);
	return f3_new(vec.x / m, vec.y / m, vec.z / m);
}

inline float f3_dst(float3 a, float3 b) { return f3_mag(f3_sub(a, b)); }

inline float3 f3_min(float3 a, float3 b) { return f3_new(MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z)); }
inline float3 f3_max(float3 a, float3 b) { return f3_new(MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z)); }

inline float3 f3_clamp(float3 a, float3 min, float3 max) { return f3_min(f3_max(a, min), max); }

inline float3 f3_neg(float3 vec) { return f3_new(-vec.x, -vec.y, -vec.z); }

#define f3_unpack(v) (v).x, (v).y, (v).z