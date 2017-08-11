#pragma once

#include <world/Chunk.h>
#include <world/WorkQueue.h>

#include <misc/NumberUtils.h>

#define CHUNKCACHE_SIZE (9)

#define UNDEADCHUNKS_COUNT ((ChunkUsage_DeaderThanDead - ChunkUsage_Undead) * CHUNKCACHE_SIZE + CHUNKCACHE_SIZE)

#define CHUNKPOOL_SIZE (CHUNKCACHE_SIZE * CHUNKCACHE_SIZE + UNDEADCHUNKS_COUNT)

typedef enum { WorldGen_Normal, WorldGen_SuperFlat } WorldGenType;
typedef struct {
	uint64_t seed;
	WorldGenType type;
	union {
		struct {
			// Keine Einstellungen...
		} superflat;
	} settings;
} GeneratorSettings;

typedef struct {
	char name[12];

	GeneratorSettings genSettings;

	int cacheTranslationX, cacheTranslationZ;

	Chunk chunkPool[CHUNKPOOL_SIZE];
	Chunk* chunkCache[CHUNKCACHE_SIZE][CHUNKCACHE_SIZE];

	WorkQueue* workqueue;
} World;

inline static int WorldToChunkCoord(int x) { return (x + (int)(x < 0)) / CHUNK_SIZE - (int)(x < 0); }
inline static int WorldToLocalCoord(int x) { return x - WorldToChunkCoord(x) * CHUNK_SIZE; }

void World_Init(World* world, WorkQueue* workqueue);

void World_Tick(World* world);

Chunk* World_LoadChunk(World* world, int x, int z);
void World_UnloadChunk(World* world, Chunk* chunk);

Chunk* World_GetChunk(World* world, int x, int z);

Block World_GetBlock(World* world, int x, int y, int z);
void World_SetBlock(World* world, int x, int y, int z, Block block);

void World_UpdateChunkCache(World* world, int orginX, int orginZ);