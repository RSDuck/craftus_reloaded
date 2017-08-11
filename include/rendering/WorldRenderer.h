#pragma once

#include <world/World.h>
#include <entity/Player.h>
#include <world/WorkQueue.h>

#include <rendering/Camera.h>

void WorldRenderer_Init(Player* player_, World* world_, WorkQueue* workqueue_, int projectionUniform_);
void WorldRenderer_Deinit();

void WorldRenderer_Render(float iod);