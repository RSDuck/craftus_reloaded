#pragma once

#include <stdint.h>

#include <world/Direction.h>

typedef uint8_t Block;

enum { Block_Air, Block_Stone, Block_Dirt, Block_Grass };

void Block_Init();
void Block_Deinit();

void* Block_GetTextureMap();

void Block_GetTexture(Block block, Direction direction, int16_t* out_uv);
