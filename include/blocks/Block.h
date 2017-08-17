#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <world/Direction.h>

typedef uint8_t Block;

enum { Block_Air,
       Block_Stone,
       Block_Dirt,
       Block_Grass,
       Block_Cobblestone,
       Block_Sand,
       Block_Log,
       Block_Leaves,
       Block_Glass,
       Block_Stonebrick,
       Block_Brick,
       Blocks_Count };

void Block_Init();
void Block_Deinit();

void* Block_GetTextureMap();

void Block_GetTexture(Block block, Direction direction, int16_t* out_uv);

uint16_t Block_GetColor(Block block, Direction direction);

bool Block_Opaque(Block block);

extern const char* BlockNames[Blocks_Count];