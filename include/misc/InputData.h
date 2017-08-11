#pragma once

#include <stdint.h>

typedef struct {
	uint32_t keysheld, keysdown, keysup;
	int16_t circlePadX, circlePadY;
	uint16_t touchX, touchY;
	int16_t cStickX, cStickY;
} InputData;