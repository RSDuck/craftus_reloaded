#include <blocks/BlockEvents.h>

void BlockEvent_RandomTick(World* world, Chunk* chunk, int x[], int y[], int z[]) {
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		for (int j = 0; j < RANDOMTICKS_PER_CLUSTER; j++) {
			int k = i * RANDOMTICKS_PER_CLUSTER + j;
			Block block = Chunk_GetBlock(chunk, x[k], y[k], z[k]);
			switch (block) {
				case Block_Dirt:
					if (Chunk_GetBlock(chunk, x[k], y[k] + 1, z[k]) == Block_Air) {
						Chunk_SetBlock(chunk, x[k], y[k], z[k], Block_Grass);
					}
					break;
				case Block_Grass:
					if (Chunk_GetBlock(chunk, x[k], y[k] + 1, z[k]) != Block_Air) {
						Chunk_SetBlock(chunk, x[k], y[k], z[k], Block_Dirt);
					}
					break;
				default:
					break;
			}
		}
	}
}