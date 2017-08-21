#pragma once

#include <stdio.h>

#include <vec/vec.h>

#include <entity/Player.h>

#include <world/savegame/SuperChunk.h>

typedef struct {
	Player* player;
	World* world;

	vec_t(SuperChunk*) superchunks;
} SaveManager;

void SaveManager_InitFileSystem();

void SaveManager_Init(SaveManager* mgr, Player* player);
void SaveManager_Deinit(SaveManager* mgr);

void SaveManager_Load(SaveManager* mgr, char* path);
void SaveManager_Unload(SaveManager* mgr);

void SaveManager_LoadChunk(WorkQueue* queue, WorkerItem item, void* this);
void SaveManager_SaveChunk(WorkQueue* queue, WorkerItem item, void* this);