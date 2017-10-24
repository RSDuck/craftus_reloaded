#include <world/World.h>

#include <blocks/BlockEvents.h>

#include <string.h>

#include <assert.h>
#include <limits.h>
#include <stdint.h>

void World_Init(World* world, WorkQueue* workqueue) {
	strcpy(world->name, "TestWelt");

	world->workqueue = workqueue;

	world->genSettings.seed = 28112000;
	world->genSettings.type = WorldGen_SuperFlat;

	vec_init(&world->freeChunks);

	World_Reset(world);
}

void World_Reset(World* world) {
	world->cacheTranslationX = 0;
	world->cacheTranslationZ = 0;

	vec_clear(&world->freeChunks);

	for (size_t i = 0; i < CHUNKPOOL_SIZE; i++) {
		world->chunkPool[i].x = INT_MAX;
		world->chunkPool[i].z = INT_MAX;
		vec_push(&world->freeChunks, &world->chunkPool[i]);
	}

	world->randomTickGen = Xorshift32_New();
}

Chunk* World_LoadChunk(World* world, int x, int z) {
	for (int i = 0; i < world->freeChunks.length; i++) {
		if (world->freeChunks.data[i]->x == x && world->freeChunks.data[i]->z == z) {
			Chunk* chunk = world->freeChunks.data[i];
			vec_splice(&world->freeChunks, i, 1);

			chunk->references++;
			return chunk;
		}
	}

	for (int i = 0; i < world->freeChunks.length; i++) {
		if (!world->freeChunks.data[i]->tasksRunning) {
			Chunk* chunk = world->freeChunks.data[i];
			vec_splice(&world->freeChunks, i, 1);

			Chunk_Init(chunk, x, z);
			WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_Load, chunk});

			chunk->references++;
			return chunk;
		}
	}

	return NULL;
}
void World_UnloadChunk(World* world, Chunk* chunk) {
	WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_Save, chunk});
	vec_push(&world->freeChunks, chunk);
	chunk->references--;
}

Chunk* World_GetChunk(World* world, int x, int z) {
	int halfS = CHUNKCACHE_SIZE / 2;
	int lowX = world->cacheTranslationX - halfS;
	int lowZ = world->cacheTranslationZ - halfS;
	int highX = world->cacheTranslationX + halfS;
	int highZ = world->cacheTranslationZ + halfS;
	if (x >= lowX && z >= lowZ && x <= highX && z <= highZ) return world->chunkCache[x - lowX][z - lowZ];
	return NULL;
}

Block World_GetBlock(World* world, int x, int y, int z) {
	if (y < 0 || y >= CHUNK_HEIGHT) return Block_Air;
	Chunk* chunk = World_GetChunk(world, WorldToChunkCoord(x), WorldToChunkCoord(z));
	if (chunk) return Chunk_GetBlock(chunk, WorldToLocalCoord(x), y, WorldToLocalCoord(z));
	return Block_Air;
}

#define NOTIFY_NEIGHTBOR(axis, comp, xDiff, zDiff)                                               \
	if (axis == comp) {                                                                      \
		Chunk* neightborChunk = World_GetChunk(world, cX + xDiff, cZ + zDiff);           \
		if (neightborChunk) Chunk_RequestGraphicsUpdate(neightborChunk, y / CHUNK_SIZE); \
	\
}

#define NOTIFY_ALL_NEIGHTBORS                                                                                             \
	NOTIFY_NEIGHTBOR(lX, 0, -1, 0)                                                                                    \
	NOTIFY_NEIGHTBOR(lX, 15, 1, 0)                                                                                    \
	NOTIFY_NEIGHTBOR(lZ, 0, 0, -1)                                                                                    \
	NOTIFY_NEIGHTBOR(lZ, 15, 0, 1)                                                                                    \
	if (WorldToLocalCoord(y) == 0 && y / CHUNK_SIZE - 1 >= 0) Chunk_RequestGraphicsUpdate(chunk, y / CHUNK_SIZE - 1); \
	if (WorldToLocalCoord(y) == 15 && y / CHUNK_SIZE + 1 < CLUSTER_PER_CHUNK) Chunk_RequestGraphicsUpdate(chunk, y / CHUNK_SIZE + 1);

void World_SetBlock(World* world, int x, int y, int z, Block block) {
	if (y < 0 || y >= CHUNK_HEIGHT) return;
	int cX = WorldToChunkCoord(x);
	int cZ = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(world, cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetBlock(chunk, lX, y, lZ, block);

		NOTIFY_ALL_NEIGHTBORS
	}
}

void World_SetBlockAndMeta(World* world, int x, int y, int z, Block block, uint8_t metadata) {
	if (y < 0 || y >= CHUNK_HEIGHT) return;
	int cX = WorldToChunkCoord(x);
	int cZ = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(world, cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetBlockAndMeta(chunk, lX, y, lZ, block, metadata);

		NOTIFY_ALL_NEIGHTBORS
	}
}

uint8_t World_GetMetadata(World* world, int x, int y, int z) {
	if (y < 0 || y >= CHUNK_HEIGHT) return 0;
	Chunk* chunk = World_GetChunk(world, WorldToChunkCoord(x), WorldToChunkCoord(z));
	if (chunk) return Chunk_GetMetadata(chunk, WorldToLocalCoord(x), y, WorldToLocalCoord(z));
	return 0;
}

void World_SetMetadata(World* world, int x, int y, int z, uint8_t metadata) {
	if (y < 0 || y >= CHUNK_HEIGHT) return;
	int cX = WorldToChunkCoord(x);
	int cZ = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(world, cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetMetadata(chunk, lX, y, lZ, metadata);

		NOTIFY_ALL_NEIGHTBORS
	}
}

int World_GetHeight(World* world, int x, int z) {
	int cX = WorldToChunkCoord(x);
	int cZ = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(world, cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);

		return Chunk_GetHeightMap(chunk, lX, lZ);
	}
	return 0;
}

void World_UpdateChunkCache(World* world, int orginX, int orginZ) {
	if (orginX != world->cacheTranslationX || orginZ != world->cacheTranslationZ) {
		Chunk* tmpBuffer[CHUNKCACHE_SIZE][CHUNKCACHE_SIZE];
		memcpy(tmpBuffer, world->chunkCache, sizeof(tmpBuffer));

		int oldBufferStartX = world->cacheTranslationX - CHUNKCACHE_SIZE / 2;
		int oldBufferStartZ = world->cacheTranslationZ - CHUNKCACHE_SIZE / 2;

		int diffX = orginX - world->cacheTranslationX;
		int diffZ = orginZ - world->cacheTranslationZ;

		for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
			for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
				int wx = orginX + (i - CHUNKCACHE_SIZE / 2);
				int wz = orginZ + (j - CHUNKCACHE_SIZE / 2);
				if (wx >= oldBufferStartX && wx < oldBufferStartX + CHUNKCACHE_SIZE && wz >= oldBufferStartZ &&
				    wz < oldBufferStartZ + CHUNKCACHE_SIZE) {
					world->chunkCache[i][j] = tmpBuffer[i + diffX][j + diffZ];
					tmpBuffer[i + diffX][j + diffZ] = NULL;
				} else {
					world->chunkCache[i][j] = World_LoadChunk(world, wx, wz);
				}
			}
		}

		for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
			for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
				if (tmpBuffer[i][j] != NULL) {
					World_UnloadChunk(world, tmpBuffer[i][j]);
				}
			}
		}

		world->cacheTranslationX = orginX;
		world->cacheTranslationZ = orginZ;
	}
}

void World_Tick(World* world) {
	for (int x = 0; x < CHUNKCACHE_SIZE; x++)
		for (int z = 0; z < CHUNKCACHE_SIZE; z++) {
			Chunk* chunk = world->chunkCache[x][z];

			if (chunk->genProgress == ChunkGen_Empty && !chunk->tasksRunning)
				WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_BaseGen, chunk});

			if (x > 0 && z > 0 && x < CHUNKCACHE_SIZE - 1 && z < CHUNKCACHE_SIZE - 1 &&
			    chunk->genProgress == ChunkGen_Terrain && !chunk->tasksRunning) {
				bool clear = true;
				for (int xOff = -1; xOff < 2 && clear; xOff++)
					for (int zOff = -1; zOff < 2 && clear; zOff++) {
						Chunk* borderChunk = world->chunkCache[x + xOff][z + zOff];
						if (borderChunk->genProgress == ChunkGen_Empty || !borderChunk->tasksRunning) clear = false;
					}
				if (clear) WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_Decorate, chunk});

				int xVals[RANDOMTICKS_PER_CHUNK];
				int yVals[RANDOMTICKS_PER_CHUNK];
				int zVals[RANDOMTICKS_PER_CHUNK];
				for (int i = 0; i < RANDOMTICKS_PER_CHUNK; i++) {
					xVals[i] = WorldToLocalCoord(Xorshift32_Next(&world->randomTickGen));
					yVals[i] = WorldToLocalCoord(Xorshift32_Next(&world->randomTickGen));
					zVals[i] = WorldToLocalCoord(Xorshift32_Next(&world->randomTickGen));
				}
				BlockEvent_RandomTick(world, chunk, xVals, yVals, zVals);
			}
		}
}