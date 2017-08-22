#pragma once

#include <world/World.h>

typedef struct { World* world; } SmeaGen;

void SmeaGen_Init(SmeaGen* gen, World* world);
void SmeaGen_Generate(WorkQueue* queue, WorkerItem item, void* this);