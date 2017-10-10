#pragma once

#include <blocks/Block.h>

#include <misc/Xorshift.h>
#include <rendering/VBOCache.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define CHUNK_SIZE (16)
#define CHUNK_HEIGHT (128)
#define CLUSTER_PER_CHUNK (CHUNK_HEIGHT / CHUNK_SIZE)

typedef struct {
	int y;
	Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	uint8_t metadataLight[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];  // first half metadata, second half light

	uint32_t revision;

	uint16_t seeThrough;

	bool empty;
	uint32_t emptyRevision;

	VBO_Block vbo, transparentVBO;
	size_t vertices, transparentVertices;
	uint32_t vboRevision;
	bool forceVBOUpdate;
} Cluster;

typedef enum {
	ChunkGen_Empty,  //
	ChunkGen_Terrain,
	ChunkGen_Finished  // Terrain | Decoration
} ChunkGenProgress;

typedef struct {
	// Die Gesamtanzahl! >= graphicalTasksRunning
	uint32_t tasksRunning;
	uint32_t graphicalTasksRunning;

	uint32_t uuid;

	ChunkGenProgress genProgress;

	int x, z;
	Cluster clusters[CLUSTER_PER_CHUNK];

	uint8_t heightmap[CHUNK_SIZE][CHUNK_SIZE];
	uint32_t heightmapRevision;

	size_t revision;

	uint32_t displayRevision;
	bool forceVBOUpdate;

	int references;
} Chunk;

extern Xorshift32 uuidGenerator;
extern const uint8_t _seethroughTable[6][6];
inline uint16_t ChunkSeeThrough(Direction in, Direction out) { return 1 << (uint16_t)(_seethroughTable[in][out]); }
inline bool ChunkCanBeSeenThrough(uint16_t visiblity, Direction in, Direction out) {
	return visiblity & (1 << (uint16_t)(_seethroughTable[in][out]));
}

inline void Chunk_Init(Chunk* chunk, int x, int z) {
	memset(chunk, 0, sizeof(Chunk));

	chunk->x = x;
	chunk->z = z;
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		chunk->clusters[i].y = i;
		chunk->clusters[i].seeThrough = UINT16_MAX;
		chunk->clusters[i].empty = true;
	}
	chunk->uuid = Xorshift32_Next(&uuidGenerator);
}

inline void Chunk_RequestGraphicsUpdate(Chunk* chunk, int cluster) {
	chunk->clusters[cluster].forceVBOUpdate = true;
	chunk->forceVBOUpdate = true;
}

void Chunk_GenerateHeightmap(Chunk* chunk);
inline uint8_t Chunk_GetHeightMap(Chunk* chunk, int x, int z) {
	Chunk_GenerateHeightmap(chunk);
	return chunk->heightmap[x][z];
}

inline uint8_t Chunk_GetMetadata(Chunk* chunk, int x, int y, int z) {
	return chunk->clusters[y / CHUNK_SIZE].metadataLight[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z] & 0xf;
}
inline void Chunk_SetMetadata(Chunk* chunk, int x, int y, int z, uint8_t metadata) {
	metadata &= 0xf;
	Cluster* cluster = &chunk->clusters[y / CHUNK_SIZE];
	uint8_t* addr = &cluster->metadataLight[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z];
	*addr = (*addr & 0xf0) | metadata;
	++cluster->revision;
	++chunk->revision;
}

inline Block Chunk_GetBlock(Chunk* chunk, int x, int y, int z) {
	return chunk->clusters[y / CHUNK_SIZE].blocks[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z];
}
// resets the meta data
inline void Chunk_SetBlock(Chunk* chunk, int x, int y, int z, Block block) {
	Cluster* cluster = &chunk->clusters[y / CHUNK_SIZE];
	cluster->blocks[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z] = block;
	Chunk_SetMetadata(chunk, x, y, z, 0);
	/*++cluster->revision;
	++chunk->revision;*/  // durch das Setzen der Metadaten wird das sowieso erhöht
}
inline void Chunk_SetBlockAndMeta(Chunk* chunk, int x, int y, int z, Block block, uint8_t metadata) {
	Cluster* cluster = &chunk->clusters[y / CHUNK_SIZE];
	cluster->blocks[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z] = block;
	metadata &= 0xf;
	uint8_t* addr = &cluster->metadataLight[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z];
	*addr = (*addr & 0xf0) | metadata;
	
	++cluster->revision;
	++chunk->revision;
}

bool Cluster_IsEmpty(Cluster* cluster);