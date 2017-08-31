#include <world/Chunk.h>

Xorshift32 uuidGenerator = (Xorshift32)314159265;

const uint8_t _seethroughTable[6][6] = {
    // W E B T N S
    {255, 0, 1, 3, 6, 10},     // West
    {0, 255, 2, 4, 7, 11},     // East
    {1, 2, 255, 5, 8, 12},     // Bottom
    {3, 4, 5, 255, 9, 13},     // Top
    {6, 7, 8, 9, 255, 14},     // North
    {10, 11, 12, 13, 14, 255}  // South
};

void Chunk_GenerateHeightmap(Chunk* chunk) {
	if (chunk->heightmapRevision != chunk->revision)
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				for (int i = CLUSTER_PER_CHUNK - 1; i >= 0; --i) {
					if (Cluster_IsEmpty(&chunk->clusters[i])) continue;
					for (int j = CHUNK_SIZE - 1; j >= 0; --j) {
						if (chunk->clusters[i].blocks[x][j][z] != Block_Air) {
							chunk->heightmap[x][z] = i * CHUNK_SIZE + j + 1;
							i = -1;
							break;
						}
					}
				}
	chunk->heightmapRevision = chunk->revision;
}

bool Cluster_IsEmpty(Cluster* cluster) {
	if (cluster->emptyRevision == cluster->revision) return cluster->empty;
	cluster->empty = false;
	cluster->emptyRevision = cluster->revision;
	for (int i = 0; i < sizeof(cluster->blocks) / sizeof(uint32_t); i++) {
		if (((uint32_t*)cluster->blocks)[i] != 0) return false;
	}
	cluster->empty = true;
	return true;
}