#pragma once

#include <world/Direction.h>
#include <world/World.h>

#include <citro3d.h>

void Cursor_Init();
void Cursor_Deinit();

void Cursor_Draw(int projUniform, C3D_Mtx* projectionview, World* world, int x, int y, int z, Direction highlight);