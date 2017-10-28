#pragma once

#include <math.h>
#include <stdbool.h>

// side effects!!!
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef CLAMP
#define CLAMP(v, min, max) MIN(MAX(v, min), max)
#endif
#ifndef ABS
#define ABS(v) ((v) < 0 ? -(v) : (v))
#endif
#ifndef BIT
#define BIT(n) (1 << (n))
#endif
#ifndef SIGN
#define SIGN(x, y) ((x) < 0 ? -(y) : (y))
#endif

static inline int FastFloor(float x) { return (int)(x) - (int)(x < (int)(x)); }
static inline float lerp(float start, float end, float t) { return start + ((end - start) * t); }
static inline float bilerp(float q11, float q21, float q12, float q22, float x, float y) { return lerp(lerp(q11, q21, x), lerp(q12, q22, x), y); }
static inline float trilerp(float q111, float q211, float q121, float q221, float q112, float q212, float q122, float q222, float x, float y, float z) {
	return lerp(bilerp(q111, q211, q112, q212, x, z), bilerp(q121, q221, q122, q222, x, z), y);
}

static inline bool AABB_Overlap(float x0, float y0, float z0, float w0, float h0, float d0, float x1, float y1, float z1, float w1, float h1, float d1) {
	return (x0 <= x1 + w1 && x0 + w0 >= x1) && (y0 <= y1 + h1 && y0 + h0 >= y1) && (z0 <= z1 + d1 && z0 + d0 >= z1);
}

#define DEG_TO_RAD (M_PI * 2.f / 360.f)
#define RAD_TO_DEG ((1.f / M_PI) * 180.f)

#ifdef _3DS
#define TICKS_PER_MSEC 268111.856
#endif