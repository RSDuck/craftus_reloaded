#pragma once

#include <c3d/texture.h>
#include <stdint.h>

void Texture_Load(C3D_Tex* result, char* filename);

#define TEXTURE_MAPSIZE 128
#define TEXTURE_TILESIZE 16
#define TEXTURE_MAPTILES (TEXTURE_MAPSIZE / TEXTURE_TILESIZE)

typedef struct {
	uint32_t textureHash;
	int16_t u, v;
} Texture_MapIcon;

typedef struct {
	C3D_Tex texture;
	Texture_MapIcon icons[TEXTURE_MAPTILES * TEXTURE_MAPTILES];
} Texture_Map;

void Texture_MapInit(Texture_Map* map, const char** files, int num_files);
Texture_MapIcon Texture_MapGetIcon(Texture_Map* map, char* filename);

void Texture_TileImage8(uint8_t* src, uint8_t* dst, int size);