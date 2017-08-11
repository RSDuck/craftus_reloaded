#pragma once

#include <vec/vec.h>

#include <world/Chunk.h>

#include <stdbool.h>

typedef enum {
	WorkerItemType_Load,
	WorkerItemType_Save,
	WorkerItemType_BaseGen,
	WorkerItemType_Decorate,
	WorkerItemType_PolyGen,
	WorkerItemTypes_Count
} WorkerItemType;

typedef struct {
	WorkerItemType type;
	Chunk* chunk;
	uint32_t uuid;
} WorkerItem;

typedef struct {
	vec_t(WorkerItem) queue[2];
	int currentQueue;
} WorkQueue;

inline void WorkQueue_Init(WorkQueue* queue) {
	vec_init(&queue->queue[0]);
	vec_init(&queue->queue[1]);
	queue->currentQueue = 0;
}
inline void WorkQueue_Deinit(WorkQueue* queue) {
	vec_deinit(&queue->queue[0]);
	vec_deinit(&queue->queue[1]);
}

inline void WorkQueue_ToggleQueue(WorkQueue* queue) { queue->currentQueue ^= 1; }

inline void WorkQueue_AddItem(WorkQueue* queue, WorkerItem item) {
	item.uuid = item.chunk->uuid;
	++item.chunk->tasksRunning;
	if (item.type == WorkerItemType_PolyGen) ++item.chunk->graphicalTasksRunning;
	vec_push(&queue->queue[queue->currentQueue], item);
}
inline WorkerItem WorkQueue_PopItem(WorkQueue* queue) { return vec_pop(&queue->queue[!queue->currentQueue]); }

inline bool WorkQueue_HasItem(WorkQueue* queue) { return queue->queue[!queue->currentQueue].length != 0; }
inline size_t WorkQueue_Empty(WorkQueue* queue) { return queue->queue[0].length == 0 && queue->queue[1].length == 0; }