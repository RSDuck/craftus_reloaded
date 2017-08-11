#include <world/worldgen/SuperFlatGen.h>

#include <misc/NumberUtils.h>

#include <sino/sino.h>

void SuperFlatGen_Init(SuperFlatGen* gen, World* world) {}

void SuperFlatGen_Generate(WorkQueue* queue, WorkerItem item, void* this) {
	SuperFlatGen* gen = this;
	for (int y = 0; y < 15; y++) {
		Block block = Block_Air;
		switch (y) {  // TODO: Mit einem Lookup Table ersetzen, Superflach Gen konfigurierbar machen
			case 0:
				block = Block_Stone;
				break;
			case 1 ... 13:
				block = Block_Dirt;
				break;
			case 14:
				block = Block_Grass;
				break;
		}
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++) Chunk_SetBlock(item.chunk, x, y, z, block);
	}
}