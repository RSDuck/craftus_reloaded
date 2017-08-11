#pragma once

#include <world/Chunk.h>

#include <vec/vec.h>

#define SUPERCHUNK_SIZE 8
#define SUPERCHUNK_BLOCKSIZE (SUPERCHUNK_SIZE * CHUNK_SIZE)

typedef struct {
	uint32_t position;
	uint32_t compressedSize;
	uint32_t actualSize;
	uint8_t blockSize;
	uint32_t revision;
} ChunkInfo;

typedef struct {
	int x, z;
	FILE* dataFile;
	ChunkInfo grid[SUPERCHUNK_SIZE][SUPERCHUNK_SIZE];
	vec_t(uint8_t) sectors;
} SuperChunk;

inline static int ChunkToSuperChunkCoord(int x) { return (x + (int)(x < 0)) / SUPERCHUNK_SIZE - (int)(x < 0); }
inline static int ChunkToLocalSuperChunkCoord(int x) { return x - ChunkToSuperChunkCoord(x) * SUPERCHUNK_SIZE; }

void SuperChunk_InitPools();
void SuperChunk_DeinitPools();

void SuperChunk_Init(SuperChunk* superchunk, int x, int z);
void SuperChunk_Deinit(SuperChunk* superchunk);
void SuperChunk_SaveIndex(SuperChunk* superchunk);

void SuperChunk_SaveChunk(SuperChunk* superchunk, Chunk* chunk);
void SuperChunk_LoadChunk(SuperChunk* superchunk, Chunk* chunk);