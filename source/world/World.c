#include <world/World.h>

#include <string.h>

#include <assert.h>

void World_Init(World* world, WorkQueue* workqueue) {
	strcpy(world->name, "TestWelt");

	world->workqueue = workqueue;

	world->genSettings.seed = 28112000;
	world->genSettings.type = WorldGen_SuperFlat;

	world->cacheTranslationX = 0;
	world->cacheTranslationZ = 0;

	for (int i = 0; i < CHUNKPOOL_SIZE; i++) world->chunkPool[i].usage = ChunkUsage_NotInUse;

	/*for (int x = 0; x < CHUNKCACHE_SIZE; x++)
		for (int z = 0; z < CHUNKCACHE_SIZE; z++) world->chunkCache[x][z] = World_LoadChunk(world, x - CHUNKCACHE_SIZE / 2, z - CHUNKCACHE_SIZE / 2);*/
}

Chunk* World_LoadChunk(World* world, int x, int z) {
	Chunk* result = NULL;
	for (int i = 0; i < CHUNKPOOL_SIZE; i++) {
		Chunk* chunk = &world->chunkPool[i];
		if (chunk->usage == ChunkUsage_NotInUse) {
			Chunk_Init(chunk, x, z);
			result = chunk;
			break;
		} else if (chunk->usage >= ChunkUsage_Undead && chunk->x == x && chunk->z == z) {
			chunk->usage = ChunkUsage_InUse;
			result = chunk;
			break;
		}
	}
	WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_Load, result});
	return result;
}
void World_UnloadChunk(World* world, Chunk* chunk) {
	chunk->usage = ChunkUsage_Undead;
	WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_Save, chunk});
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
void World_SetBlock(World* world, int x, int y, int z, Block block) {
	if (y < 0 || y >= CHUNK_HEIGHT) return;
	int cX = WorldToChunkCoord(x);
	int cZ = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(world, cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetBlock(chunk, lX, y, lZ, block);

#define NOTIFY_NEIGHTBOR(axis, comp, xDiff, zDiff)                                       \
	if (axis == comp) {                                                              \
		Chunk* neightborChunk = World_GetChunk(world, cX + xDiff, cZ + zDiff);   \
		if (neightborChunk) Chunk_RequestGraphicsUpdate(neightborChunk, y / CHUNK_SIZE); \
	}
		NOTIFY_NEIGHTBOR(lX, 0, -1, 0)
		NOTIFY_NEIGHTBOR(lX, 15, 1, 0)
		NOTIFY_NEIGHTBOR(lZ, 0, 0, -1)
		NOTIFY_NEIGHTBOR(lZ, 15, 0, 1)

		if (WorldToLocalCoord(y) == 0 && y / CHUNK_SIZE - 1 >= 0) Chunk_RequestGraphicsUpdate(chunk, y / CHUNK_SIZE - 1);
		if (WorldToLocalCoord(y) == 15 && y / CHUNK_SIZE + 1 < CLUSTER_PER_CHUNK) Chunk_RequestGraphicsUpdate(chunk, y / CHUNK_SIZE + 1);
	}
}

static void World_ClearUndeadChunks(World* world) {
	for (int i = 0; i < CHUNKPOOL_SIZE; i++)
		if (world->chunkPool[i].usage >= ChunkUsage_Undead && ++world->chunkPool[i].usage >= ChunkUsage_DeaderThanDead) {
			world->chunkPool[i].usage = ChunkUsage_NotInUse;
			printf("Deleted chunk\n");
		}
}

void World_UpdateChunkCache(World* world, int orginX, int orginZ) {
	if (orginX != world->cacheTranslationX || orginZ != world->cacheTranslationZ) {
		World_ClearUndeadChunks(world);

		int diffX = orginX - world->cacheTranslationX;
		int diffZ = orginZ - world->cacheTranslationZ;

		int axis = 0;

#define CACHE_AXIS world->chunkCache[!axis ? i : j][!axis ? j : i]

		// Tick
		int delta = diffX;
		do {
			if (delta != 0) {
				if (delta > 0) {
					for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
						for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
							if (i == 0) World_UnloadChunk(world, CACHE_AXIS);
							if (i + delta >= CHUNKCACHE_SIZE)
								CACHE_AXIS = World_LoadChunk(world, CACHE_AXIS->x + (!axis ? delta : 0),
											     CACHE_AXIS->z + (!axis ? 0 : delta));
							else
								CACHE_AXIS = world->chunkCache[!axis ? (i + delta) : j][!axis ? j : (i + delta)];
						}
					}
				} else {
					for (int i = CHUNKCACHE_SIZE - 1; i >= 0; i--) {
						for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
							if (i == CHUNKCACHE_SIZE - 1) World_UnloadChunk(world, CACHE_AXIS);
							if (i + delta < 0)
								CACHE_AXIS = World_LoadChunk(world, CACHE_AXIS->x + (!axis ? delta : 0),
											     CACHE_AXIS->z + (!axis ? 0 : delta));
							else
								CACHE_AXIS = world->chunkCache[!axis ? (i + delta) : j][!axis ? j : (i + delta)];
						}
					}
				}
			}
			// Tock
			delta = diffZ;
		} while (++axis < 2);

#undef CACHE_AXIS
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

			if (x > 0 && z > 0 && x < CHUNKCACHE_SIZE - 1 && z < CHUNKCACHE_SIZE - 1 && chunk->genProgress == ChunkGen_Terrain &&
			    !chunk->tasksRunning) {
				bool clear = true;
				for (int xOff = -1; xOff < 2 && clear; xOff++)
					for (int zOff = -1; zOff < 2 && clear; zOff++) {
						Chunk* borderChunk = world->chunkCache[x + xOff][z + zOff];
						if (borderChunk->genProgress == ChunkGen_Empty || !borderChunk->tasksRunning) clear = false;
					}
				if (clear) WorkQueue_AddItem(world->workqueue, (WorkerItem){WorkerItemType_Decorate, chunk});
			}
		}
}