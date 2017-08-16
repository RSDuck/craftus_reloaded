#pragma once

#include <blocks/Block.h>
#include <world/World.h>

#define RANDOMTICKS_PER_CLUSTER (3)
#define RANDOMTICKS_PER_CHUNK (CLUSTER_PER_CHUNK * RANDOMTICKS_PER_CLUSTER)
// #x = #y = #z = RANDOMTICKS_PER_CHUNK
void BlockEvent_RandomTick(World* world, Chunk* chunk, int x[], int y[], int z[]);