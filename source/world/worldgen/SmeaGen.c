#include <world/worldgen/SmeaGen.h>

#include <sino/sino.h>

void SmeaGen_Init(SmeaGen* gen, World* world) { gen->world = world; }

// based off https://github.com/smealum/3dscraft/blob/master/source/generation.c
void SmeaGen_Generate(WorkQueue* queue, WorkerItem item, void* this) {
	SmeaGen* gen = this;
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			float px = (float)(x + item.chunk->x * CHUNK_SIZE);
			float pz = (float)(z + item.chunk->z * CHUNK_SIZE);

			const int smeasClusterSize = 8;
			const int smeasChunkHeight = 16;
			int height = (int)(sino_2d((px) / (smeasClusterSize * 4), (pz) / (smeasClusterSize * 4)) * smeasClusterSize) +
				     (smeasChunkHeight * smeasClusterSize / 2);

			for (int y = 0; y < height - 3; y++) {
				Chunk_SetBlock(item.chunk, x, y, z, Block_Stone);
			}
			for (int y = height - 3; y < height; y++) {
				Chunk_SetBlock(item.chunk, x, y, z, Block_Dirt);
			}
			Chunk_SetBlock(item.chunk, x, height, z, Block_Grass);
		}
	}
}