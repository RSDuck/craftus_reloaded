#pragma once

#include <world/WorkQueue.h>
#include <world/World.h>

#include <rendering/VertexFmt.h>

void PolyGen_Init(World* world_);
void PolyGen_Deinit();

void PolyGen_Harvest();
void PolyGen_GeneratePolygons(WorkQueue* queue, WorkerItem item, void* this);

void PolyGen_Lock();
void PolyGen_Unlock();