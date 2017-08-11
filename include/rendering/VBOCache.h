#pragma once

#include <stdio.h>

void VBOCache_Init();
void VBOCache_Deinit();

typedef struct {
	size_t size;
	void* memory;
} VBO_Block;

VBO_Block VBO_Alloc(size_t size);
void VBO_Free(VBO_Block block);