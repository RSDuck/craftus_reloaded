#pragma once

#include <citro3d.h>

#include <world/World.h>

void Clouds_Init();
void Clouds_Deinit();

void Clouds_Render(int projUniform, C3D_Mtx* projectionview, World* world, float tx, float tz);