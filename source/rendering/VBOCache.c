#include <rendering/VBOCache.h>

#include <vec/vec.h>

#include <3ds.h>

static vec_t(VBO_Block) freedBlocks;

static LightLock lock;

void VBOCache_Init() {
	vec_init(&freedBlocks);
	LightLock_Init(&lock);
}
void VBOCache_Deinit() {
	VBO_Block block;
	int i;
	vec_foreach (&freedBlocks, block, i) { linearFree(block.memory); }
	vec_deinit(&freedBlocks);
}

VBO_Block VBO_Alloc(size_t size) {
	LightLock_Lock(&lock);
	if (freedBlocks.length > 0) {
		VBO_Block block;
		int i;
		vec_foreach (&freedBlocks, block, i) {
			if (size <= block.size && block.size - size <= 2048) {
				vec_splice(&freedBlocks, i, 1);
				LightLock_Unlock(&lock);
				return block;
			}
		}
	}
	VBO_Block block;
	block.memory = linearAlloc(size);
	block.size = size;
	LightLock_Unlock(&lock);
	return block;
}

static int sort_by_size(const void* a, const void* b) { return ((VBO_Block*)b)->size - ((VBO_Block*)a)->size; }

void VBO_Free(VBO_Block block) {
	if (block.size > 0 && block.memory != NULL) {
		LightLock_Lock(&lock);
		vec_push(&freedBlocks, block);
		vec_sort(&freedBlocks, &sort_by_size);
		LightLock_Unlock(&lock);
	}
}
