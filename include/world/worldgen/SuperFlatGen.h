#pragma once

#include <world/WorkQueue.h>
#include <world/World.h>

typedef struct { World* world; } SuperFlatGen;

void SuperFlatGen_Init(SuperFlatGen* gen, World* world);

void SuperFlatGen_Generate(WorkQueue* queue, WorkerItem item, void* this);
