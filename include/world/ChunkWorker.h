#pragma once

#include <3ds.h>

#include <world/WorkQueue.h>

#define CHUNKWORKER_THREAD_STACKSIZE (16 * 1024)

typedef void (*WorkerFunc)(WorkQueue* queue, WorkerItem item, void* this);
typedef struct {
	WorkerFunc func;
	void* this;
	bool active;
} WorkerFuncObj;

typedef struct {
	Thread thread;

	WorkQueue queue;

	vec_t(WorkerFuncObj) handler[WorkerItemTypes_Count];

	volatile bool working;
} ChunkWorker;

void ChunkWorker_Init(ChunkWorker* chunkworker);
void ChunkWorker_Deinit(ChunkWorker* chunkworker);

void ChunkWorker_Finish(ChunkWorker* chunkworker);

void ChunkWorker_AddHandler(ChunkWorker* chunkworker, WorkerItemType type, WorkerFuncObj obj);
void ChunkWorker_SetHandlerActive(ChunkWorker* chunkworker, WorkerItemType type, void* this, bool active);

void ChunkWorker_Mainloop(void* _this);