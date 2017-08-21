#pragma once

#include <entity/Player.h>
#include <world/WorkQueue.h>
#include <world/World.h>
#include <GameStates.h>

// Der Renderer ist ein Singleton, weil die darunterliegende Graphik API es auch ist
void Renderer_Init(World* world_, Player* player_, WorkQueue* queue, GameState* gamestate_);
void Renderer_Deinit();

void Renderer_Render();