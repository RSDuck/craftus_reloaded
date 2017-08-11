#pragma once

#include <stdint.h>

typedef struct {
	int16_t xyz[3];
	int16_t uvc[3];
} Vertex;

#define SHADER_RGB(r,g,b) ((b) | ((g) << 5) | ((r) << 10))