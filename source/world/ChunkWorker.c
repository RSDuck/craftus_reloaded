#include <world/ChunkWorker.h>

#include <stdio.h>

void ChunkWorker_Init(ChunkWorker* chunkworker) {
	WorkQueue_Init(&chunkworker->queue);

	if (R_FAILED(APT_SetAppCpuTimeLimit(30))) {
		printf("Couldn't set AppCpuTimeLimit\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < WorkerItemTypes_Count; i++) vec_init(&chunkworker->handler[i]);

	int prio;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	chunkworker->thread = threadCreate(&ChunkWorker_Mainloop, (void*)chunkworker, CHUNKWORKER_THREAD_STACKSIZE, /*prio - 1*/ 0x3f, 1, false);
	if (!chunkworker->thread) {
		printf("Couldn't create worker thread\n");
		exit(EXIT_FAILURE);
	}
}

static volatile ChunkWorker* workerToStop = NULL;
void ChunkWorker_Deinit(ChunkWorker* chunkworker) {
	workerToStop = chunkworker;
	threadJoin(chunkworker->thread, UINT64_MAX);

	threadFree(chunkworker->thread);
	WorkQueue_Deinit(&chunkworker->queue);

	for (int i = 0; i < WorkerItemTypes_Count; i++) {
		vec_deinit(&chunkworker->handler[i]);
	}
}

void ChunkWorker_AddHandler(ChunkWorker* chunkworker, WorkerItemType type, WorkerFuncObj obj) { vec_push(&chunkworker->handler[type], obj); }

void ChunkWorker_Mainloop(void* _this) {
	// printf("ChunkWorker_Mainloop\n");
	ChunkWorker* chunkworker = (ChunkWorker*)_this;
	while (workerToStop != chunkworker || !WorkQueue_Empty(&chunkworker->queue)) {
		WorkQueue_ToggleQueue(&chunkworker->queue);
		if (WorkQueue_HasItem(&chunkworker->queue)) {
			WorkerItem item = WorkQueue_PopItem(&chunkworker->queue);

			if (item.uuid == item.chunk->uuid && (workerToStop == chunkworker ? item.type == WorkerItemType_Save : true)) {
				for (int i = 0; i < chunkworker->handler[item.type].length; i++) {
					chunkworker->handler[item.type].data[i].func(&chunkworker->queue, item, chunkworker->handler[item.type].data[i].this);
					svcSleepThread(300);
				}

				switch (item.type) {
					case WorkerItemType_BaseGen:
						item.chunk->genProgress = ChunkGen_Terrain;
						break;
					case WorkerItemType_Decorate:
						item.chunk->genProgress = ChunkGen_Finished;
						break;
					default:
						break;
				}
				svcSleepThread(1000);

				--item.chunk->tasksRunning;
				if (item.type == WorkerItemType_PolyGen) --item.chunk->graphicalTasksRunning;
			}
		} else
			svcSleepThread(25000);
	}

	printf("ChunkWorker_Mainloop End");
}