#include <rendering/WorldRenderer.h>

#include <rendering/VertexFmt.h>

#include <rendering/Cursor.h>
#include <blocks/Block.h>

#include <gui/DebugUI.h>

#include <citro3d.h>

#include <rendering/Clouds.h>
#include <rendering/Hand.h>

static Player* player;
static World* world;

static WorkQueue* workqueue;

static Camera camera;

static int projectionUniform;

typedef struct {
	Cluster* cluster;
	Chunk* chunk;
	Direction enteredFrom;
} RenderStep;

typedef struct {
	Cluster* cluster;
	Chunk* chunk;
} TransparentRender;

static vec_t(RenderStep) renderingQueue;
static uint8_t chunkRendered[CHUNKCACHE_SIZE][CLUSTER_PER_CHUNK][CHUNKCACHE_SIZE];
static vec_t(TransparentRender) transparentClusters;

static C3D_FogLut fogLut;

#define clusterWasRendered(x, y, z) \
	chunkRendered[x - (world->cacheTranslationX - (CHUNKCACHE_SIZE / 2))][y][z - (world->cacheTranslationZ - (CHUNKCACHE_SIZE / 2))]

void WorldRenderer_Init(Player* player_, World* world_, WorkQueue* workqueue_, int projectionUniform_) {
	world = world_;
	player = player_;
	projectionUniform = projectionUniform_;
	workqueue = workqueue_;

	vec_init(&renderingQueue);
	vec_init(&transparentClusters);

	Camera_Init(&camera);

	Cursor_Init();
	Hand_Init();

	float data[256];
	for (int i = 0; i <= 128; i++) {
		float val = 1.f;
		if (i == 0)
			val = 0.0f;
		else if (i == 1)
			val = 0.9f;
		if (i < 128) data[i] = val;
		if (i > 0) data[i + 127] = val - data[i - 1];
	}
	FogLut_FromArray(&fogLut, data);
	C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
	C3D_FogColor(0xffd990);
	C3D_FogLutBind(&fogLut);

	Clouds_Init();
}
void WorldRenderer_Deinit() {
	vec_deinit(&renderingQueue);
	vec_deinit(&transparentClusters);
	Cursor_Deinit();

	Hand_Deinit();

	Clouds_Deinit();
}

static void renderWorld() {
	C3D_FogColor(0xffd990);

	memset(chunkRendered, 0, sizeof(chunkRendered));

	int polysTotal = 0, clustersDrawn = 0, steps = 0;

	vec_clear(&renderingQueue);
	vec_clear(&transparentClusters);

	int pY = CLAMP(WorldToChunkCoord(FastFloor(player->position.y)), 0, CLUSTER_PER_CHUNK - 1);
	Chunk* pChunk =
	    World_GetChunk(world, WorldToChunkCoord(FastFloor(player->position.x)), WorldToChunkCoord(FastFloor(player->position.z)));
	vec_push(&renderingQueue, ((RenderStep){&pChunk->clusters[pY], pChunk, Direction_Invalid}));
	chunkRendered[CHUNKCACHE_SIZE / 2][pY][CHUNKCACHE_SIZE / 2] = 1;

	float3 playerPos = player->position;

	while (renderingQueue.length > 0) {
		RenderStep step = vec_pop(&renderingQueue);
		Chunk* chunk = step.chunk;
		Cluster* cluster = step.cluster;

		if (cluster->vertices > 0 && cluster->vbo.size) {
			clusterWasRendered(chunk->x, cluster->y, chunk->z) |= 2;

			C3D_BufInfo bufInfo;
			BufInfo_Init(&bufInfo);
			BufInfo_Add(&bufInfo, cluster->vbo.memory, sizeof(WorldVertex), 4, 0x3210);
			C3D_SetBufInfo(&bufInfo);
			C3D_DrawArrays(GPU_TRIANGLES, 0, cluster->vertices);

			polysTotal += cluster->vertices;

			clustersDrawn++;
		}
		if (cluster->transparentVertices > 0 && cluster->transparentVBO.size) {
			vec_push(&transparentClusters, ((TransparentRender){cluster, chunk}));
		}
		// if (polysTotal >= 150000) break;

		for (int i = 0; i < 6; i++) {
			Direction dir = i;
			const int* offset = DirectionToOffset[dir];

			int newX = chunk->x + offset[0], newY = cluster->y + offset[1], newZ = chunk->z + offset[2];
			if (newX < world->cacheTranslationX - CHUNKCACHE_SIZE / 2 + 1 ||
			    newX > world->cacheTranslationX + CHUNKCACHE_SIZE / 2 - 1 ||
			    newZ < world->cacheTranslationZ - CHUNKCACHE_SIZE / 2 + 1 ||
			    newZ > world->cacheTranslationZ + CHUNKCACHE_SIZE / 2 - 1 || newY < 0 || newY >= CLUSTER_PER_CHUNK)
				continue;
			float3 dist = f3_sub(f3_new(newX * CHUNK_SIZE + CHUNK_SIZE / 2, newY * CHUNK_SIZE + CHUNK_SIZE / 2,
						    newZ * CHUNK_SIZE + CHUNK_SIZE / 2),
					     playerPos);
			if (f3_dot(dist, dist) > (3.f * CHUNK_SIZE) * (3.f * CHUNK_SIZE)) {
				continue;
			}

			if (clusterWasRendered(newX, newY, newZ) & 1) continue;

			if (!ChunkCanBeSeenThrough(cluster->seeThrough, step.enteredFrom, i) && step.enteredFrom != Direction_Invalid)
				continue;

			C3D_FVec chunkPosition = FVec3_New(newX * CHUNK_SIZE, newY * CHUNK_SIZE, newZ * CHUNK_SIZE);
			if (!Camera_IsAABBVisible(&camera, chunkPosition, FVec3_New(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE))) continue;

			clusterWasRendered(newX, newY, newZ) |= 1;

			Chunk* newChunk = World_GetChunk(world, newX, newZ);
			RenderStep nextStep = (RenderStep){&newChunk->clusters[newY], newChunk, DirectionOpposite[dir]};
			if (newChunk) vec_push(&renderingQueue, nextStep);
		}
	}

	for (int x = 1; x < CHUNKCACHE_SIZE - 1; x++) {
		for (int z = 1; z < CHUNKCACHE_SIZE - 1; z++) {
			Chunk* chunk = world->chunkCache[x][z];

			if ((chunk->revision != chunk->displayRevision || chunk->forceVBOUpdate) && !chunk->tasksRunning) {
				bool clear = true;
				for (int xOff = -1; xOff < 2 && clear; xOff++)
					for (int zOff = -1; zOff < 2 && clear; zOff++)
						if (world->chunkCache[x + xOff][z + zOff]->genProgress == ChunkGen_Empty) clear = false;

				if (clear) WorkQueue_AddItem(workqueue, (WorkerItem){WorkerItemType_PolyGen, chunk});
			}
		}
	}

	C3D_AlphaTest(true, GPU_GEQUAL, 255);

	int i;
	TransparentRender* render;
	vec_foreach_ptr_rev(&transparentClusters, render, i) {
		C3D_BufInfo bufInfo;
		BufInfo_Init(&bufInfo);
		BufInfo_Add(&bufInfo, render->cluster->transparentVBO.memory, sizeof(WorldVertex), 4, 0x3210);
		C3D_SetBufInfo(&bufInfo);
		C3D_DrawArrays(GPU_TRIANGLES, 0, render->cluster->transparentVertices);

		polysTotal += render->cluster->transparentVertices;
	}
	C3D_AlphaTest(false, GPU_GREATER, 0);

	DebugUI_Text("Clusters drawn %d with %d steps. %d vertices", clustersDrawn, steps, polysTotal);
	DebugUI_Text("T: %u P: %u %d", world->chunkCache[CHUNKCACHE_SIZE / 2][CHUNKCACHE_SIZE / 2]->tasksRunning,
		     world->chunkCache[CHUNKCACHE_SIZE / 2][CHUNKCACHE_SIZE / 2]->genProgress, workqueue->queue.length);
}

void WorldRenderer_Render(float iod) {
	Camera_Update(&camera, player, iod);

	Hand_Draw(projectionUniform, &camera.projection, player->quickSelectBar[player->quickSelectBarSlot], player);
	C3D_TexBind(0, Block_GetTextureMap());

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projectionUniform, &camera.vp);

	renderWorld();

	Clouds_Render(projectionUniform, &camera.vp, world, player->position.x, player->position.z);

	if (player->blockInActionRange)
		Cursor_Draw(projectionUniform, &camera.vp, world, player->viewRayCast.x, player->viewRayCast.y, player->viewRayCast.z,
			    player->viewRayCast.direction);
}