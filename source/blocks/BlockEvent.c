#include <blocks/BlockEvents.h>

void BlockEvent_RandomTick(World* world, Chunk* chunk, int x[], int y[], int z[]) {
	int k = 0;
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		for (int j = 0; j < RANDOMTICKS_PER_CLUSTER; j++) {
			int px = x[k];
			int py = y[k] + i * CHUNK_SIZE;
			int pz = z[k];
			Block block = Chunk_GetBlock(chunk, px, py, pz);
			switch (block) {
				case Block_Dirt:
					if (!Block_Opaque(Chunk_GetBlock(chunk, px, py + 1, pz),
							  Chunk_GetMetadata(chunk, px, py + 1, pz))) {
						Chunk_SetBlock(chunk, px, py, pz, Block_Grass);
					}
					break;
				case Block_Grass:
					if (Block_Opaque(Chunk_GetBlock(chunk, px, py + 1, pz), Chunk_GetMetadata(chunk, px, py + 1, pz))) {
						Chunk_SetBlock(chunk, px, py, pz, Block_Dirt);
					}
					break;
				default:
					break;
			}
			k++;
		}
	}
}