#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>

#include <GameStates.h>
#include <entity/Player.h>
#include <entity/PlayerController.h>
#include <gui/DebugUI.h>
#include <gui/Gui.h>
#include <gui/WorldSelect.h>
#include <rendering/PolyGen.h>
#include <rendering/Renderer.h>
#include <world/ChunkWorker.h>
#include <world/World.h>
#include <world/savegame/SaveManager.h>
#include <world/savegame/SuperChunk.h>
#include <world/worldgen/SmeaGen.h>
#include <world/worldgen/SuperFlatGen.h>

#include <sino/sino.h>

#include <citro3d.h>

bool showDebugInfo = false;  // muss noch besser gemacht werden, vlt. über eine Options Struktur wo auch andere Einstellungen drinne sind

void releaseWorld(ChunkWorker* chunkWorker, SaveManager* savemgr, World* world) {
	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			World_UnloadChunk(world, world->chunkCache[i][j]);
		}
	}
	ChunkWorker_Finish(chunkWorker);
	World_Reset(world);

	SaveManager_Unload(savemgr);
}

int main() {
	GameState gamestate = GameState_SelectWorld;

	gfxInitDefault();

	gfxSet3D(true);

	romfsInit();

	SuperFlatGen flatGen;
	SmeaGen smeaGen;

	SuperChunk_InitPools();

	SaveManager_InitFileSystem();

	ChunkWorker chunkWorker;
	ChunkWorker_Init(&chunkWorker);
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_PolyGen, (WorkerFuncObj){&PolyGen_GeneratePolygons, NULL, true});
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){&SuperFlatGen_Generate, &flatGen, true});
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){&SmeaGen_Generate, &smeaGen, true});

	sino_init();

	World* world = (World*)malloc(sizeof(World));
	Player player;
	PlayerController playerCtrl;
	Player_Init(&player, world);
	PlayerController_Init(&playerCtrl, &player);

	SuperFlatGen_Init(&flatGen, world);
	SmeaGen_Init(&smeaGen, world);

	Renderer_Init(world, &player, &chunkWorker.queue, &gamestate);

	DebugUI_Init();

	WorldSelect_Init();

	World_Init(world, &chunkWorker.queue);

	SaveManager savemgr;
	SaveManager_Init(&savemgr, &player);
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_Load, (WorkerFuncObj){&SaveManager_LoadChunk, &savemgr, true});
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_Save, (WorkerFuncObj){&SaveManager_SaveChunk, &savemgr, true});

	uint64_t lastTime = svcGetSystemTick();
	float dt = 0.f, timeAccum = 0.f, fpsClock = 0.f;
	int frameCounter = 0, fps = 0;
	while (aptMainLoop()) {
		DebugUI_Text("%d FPS  Usage: CPU: %5.2f%% GPU: %5.2f%% Buf: %5.2f%% Lin: %d", fps, C3D_GetProcessingTime() * 6.f,
			     C3D_GetDrawingTime() * 6.f, C3D_GetCmdBufUsage() * 100.f, linearSpaceFree());
		DebugUI_Text("Player: %f, %f, %f P: %f Y: %f", f3_unpack(player.position), player.pitch, player.yaw);

		Renderer_Render();

		uint64_t currentTime = svcGetSystemTick();
		dt = ((float)(currentTime / (float)TICKS_PER_MSEC) - (float)(lastTime / (float)TICKS_PER_MSEC)) / 1000.f;
		lastTime = currentTime;
		timeAccum += dt;

		frameCounter++;
		fpsClock += dt;
		if (fpsClock >= 1.f) {
			fps = frameCounter;
			frameCounter = 0;
			fpsClock = 0.f;
		}

		hidScanInput();
		u32 keysheld = hidKeysHeld(), keysdown = hidKeysDown();
		if (keysdown & KEY_START) {
			if (gamestate == GameState_SelectWorld)
				break;
			else if (gamestate == GameState_Playing) {
				releaseWorld(&chunkWorker, &savemgr, world);

				gamestate = GameState_SelectWorld;

				WorldSelect_ScanWorlds();

				lastTime = svcGetSystemTick();
			}
		}

		circlePosition circlePos;
		hidCircleRead(&circlePos);

		circlePosition cstickPos;
		hidCstickRead(&cstickPos);

		touchPosition touchPos;
		hidTouchRead(&touchPos);

		InputData inputData = (InputData){keysheld,    keysdown,    hidKeysUp(),  circlePos.dx, circlePos.dy,
						  touchPos.px, touchPos.py, cstickPos.dx, cstickPos.dy};

		if (gamestate == GameState_Playing) {
			while (timeAccum >= 1.f / 20.f) {
				World_Tick(world);

				timeAccum -= 1.f / 20.f;
			}

			PlayerController_Update(&playerCtrl, inputData, dt);

			World_UpdateChunkCache(world, WorldToChunkCoord(FastFloor(player.position.x)),
					       WorldToChunkCoord(FastFloor(player.position.z)));
		} else if (gamestate == GameState_SelectWorld) {
			char path[256];
			char name[WORLD_NAME_SIZE] = {'\0'};
			WorldGenType worldType;
			bool newWorld = false;
			if (WorldSelect_Update(path, name, &worldType, &newWorld)) {
				strcpy(world->name, name);
				world->genSettings.type = worldType;

				SaveManager_Load(&savemgr, path);

				ChunkWorker_SetHandlerActive(&chunkWorker, WorkerItemType_BaseGen, &flatGen,
							     world->genSettings.type == WorldGen_SuperFlat);
				ChunkWorker_SetHandlerActive(&chunkWorker, WorkerItemType_BaseGen, &smeaGen,
							     world->genSettings.type == WorldGen_Smea);

				world->cacheTranslationX = WorldToChunkCoord(FastFloor(player.position.x));
				world->cacheTranslationZ = WorldToChunkCoord(FastFloor(player.position.z));
				for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
					for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
						world->chunkCache[i][j] =
						    World_LoadChunk(world, i - CHUNKCACHE_SIZE / 2 + world->cacheTranslationX,
								    j - CHUNKCACHE_SIZE / 2 + world->cacheTranslationZ);
					}
				}

				for (int i = 0; i < 3; i++) {
					while (chunkWorker.working || chunkWorker.queue.queue.length > 0) {
						svcSleepThread(50000000);  // 1 Tick
					}
					World_Tick(world);
				}

				if (newWorld) {
					int highestBlock = 0;
					for (int x = -1; x < 1; x++) {
						for (int z = -1; z < 1; z++) {
							int height = World_GetHeight(world, x, z);
							if (height > highestBlock) highestBlock = height;
						}
					}
					player.position.y = (float)highestBlock + 0.2f;
				}

				gamestate = GameState_Playing;
				lastTime = svcGetSystemTick();  // fix timing
			}
		}
		Gui_InputData(inputData);
	}

	if (gamestate == GameState_Playing) releaseWorld(&chunkWorker, &savemgr, world);


	SaveManager_Deinit(&savemgr);

	SuperChunk_DeinitPools();

	free(world);

	sino_exit();

	WorldSelect_Deinit();

	DebugUI_Deinit();

	ChunkWorker_Deinit(&chunkWorker);

	Renderer_Deinit();

	romfsExit();

	gfxExit();
	return 0;
}
