#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>

#include <entity/Player.h>
#include <entity/PlayerController.h>
#include <gui/DebugUI.h>
#include <gui/Gui.h>
#include <rendering/PolyGen.h>
#include <rendering/Renderer.h>
#include <world/ChunkWorker.h>
#include <world/World.h>
#include <world/savegame/SaveManager.h>
#include <world/savegame/SuperChunk.h>
#include <world/worldgen/SuperFlatGen.h>

#include <citro3d.h>

void exitHandler() {
	printf("Fatal error, press start to exit\n");
	while (aptMainLoop()) {
		gspWaitForVBlank();

		hidScanInput();

		if (hidKeysDown() & KEY_START) break;
	}
}

int main() {
	gfxInitDefault();

	/*PrintConsole consoleEvent, consoleStatus;
	consoleInit(GFX_BOTTOM, &consoleEvent);
	consoleInit(GFX_BOTTOM, &consoleStatus);

	consoleSetWindow(&consoleStatus, 0, 0, 320 / 8, 240 / 16);
	consoleSetWindow(&consoleEvent, 0, 240 / 16 + 1, 320 / 8, 240 / 16 - 1);*/

	gfxSet3D(true);

	romfsInit();

	atexit(&exitHandler);

	SuperFlatGen flatGen;

	SuperChunk_InitPools();

	ChunkWorker chunkWorker;
	ChunkWorker_Init(&chunkWorker);
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_PolyGen, (WorkerFuncObj){&PolyGen_GeneratePolygons, NULL});
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){&SuperFlatGen_Generate, &flatGen});

	World* world = (World*)malloc(sizeof(World));
	Player player;
	PlayerController playerCtrl;
	Player_Init(&player);
	player.position.y = 40.f;
	Player_Spawn(&player, world);
	PlayerController_Init(&playerCtrl, &player);

	SuperFlatGen_Init(&flatGen, world);

	Renderer_Init(world, &player, &chunkWorker.queue);

	DebugUI_Init();

	World_Init(world, &chunkWorker.queue);
	
	
	//consoleInit(GFX_BOTTOM, NULL);

	SaveManager savemgr;
	SaveManager_Init(&savemgr, &player);
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_Load, (WorkerFuncObj){&SaveManager_LoadChunk, &savemgr});
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_Save, (WorkerFuncObj){&SaveManager_SaveChunk, &savemgr});

	world->cacheTranslationX = WorldToChunkCoord(FastFloor(player.position.x));
	world->cacheTranslationZ = WorldToChunkCoord(FastFloor(player.position.z));
	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			world->chunkCache[i][j] =
			    World_LoadChunk(world, i - CHUNKCACHE_SIZE / 2 + world->cacheTranslationX, j - CHUNKCACHE_SIZE / 2 + world->cacheTranslationZ);
		}
	}

	while (chunkWorker.queue.queue[0].length > 0 || chunkWorker.queue.queue[1].length > 0) {
		svcSleepThread(4800000);
	}

	uint64_t lastTime = svcGetSystemTick();
	float dt = 0.f, timeAccum = 0.f, fpsClock = 0.f;
	int frameCounter = 0, fps = 0;
	while (aptMainLoop()) {
		DebugUI_Text("%d FPS  Usage: CPU: %5.2f%% GPU: %5.2f%% Buf: %5.2f%% Lin: %d", fps, C3D_GetProcessingTime() * 6.f, C3D_GetDrawingTime() * 6.f,
			     C3D_GetCmdBufUsage() * 100.f, linearSpaceFree());
		DebugUI_Text("Player: %f, %f, %f", f3_unpack(player.position));

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

		while (timeAccum >= 1.f / 20.f) {
			World_Tick(world);

			timeAccum -= 1.f / 20.f;
		}

		hidScanInput();
		u32 keysdown = hidKeysHeld();
		if (keysdown & KEY_START) break;

		circlePosition circlePos;
		hidCircleRead(&circlePos);

		touchPosition touchPos;
		hidTouchRead(&touchPos);

		InputData inputData = (InputData){keysdown, hidKeysDown(), hidKeysUp(), circlePos.dx, circlePos.dy, touchPos.px, touchPos.py};
		PlayerController_Update(&playerCtrl, inputData, dt);

		World_UpdateChunkCache(world, WorldToChunkCoord(FastFloor(player.position.x)), WorldToChunkCoord(FastFloor(player.position.z)));

		Gui_InputData(inputData);
	}

	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			World_UnloadChunk(world, world->chunkCache[i][j]);
		}
	}

	ChunkWorker_Deinit(&chunkWorker);

	SaveManager_Deinit(&savemgr);

	SuperChunk_DeinitPools();

	free(world);

	DebugUI_Deinit();

	Renderer_Deinit();

	romfsExit();

	gfxExit();
	return 0;
}
