#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <world/World.h>

typedef struct {
    char worldName[WORLD_NAME_SIZE];
    GeneratorSettings settings;    
} WorldSelect_Result;

void WorldSelect_Init();
void WorldSelect_Deinit();

void WorldSelect_Render();
bool WorldSelect_Update(char* out_worldpath, char* out_name, WorldGenType* worldType, bool* newWorld);

void WorldSelect_ScanWorlds();