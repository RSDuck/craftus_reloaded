#pragma once

#include <stdint.h>

typedef struct {
	int16_t xyz[3];
	int16_t uvc[3];
} Vertex;

#define SHADER_RGB(r, g, b) (((b & 0x1f)) | (((g) & 0x1f) << 5) | (((r) & 0x1f) << 10))

#define SHADER_R(r) (((r) >> 10) & 0x1f)
#define SHADER_G(g) (((g) >> 5) & 0x1f)
#define SHADER_B(b) ((b)& 0x1f)
#define SHADER_RGB_MIX(a, b) SHADER_RGB(SHADER_R(a) / 2 + SHADER_R(b) / 2, SHADER_G(a) / 2 + SHADER_G(b) / 2, SHADER_B(a) / 2 + SHADER_B(b) / 2)