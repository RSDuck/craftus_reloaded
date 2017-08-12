#include <rendering/PolyGen.h>

#include <gui/DebugUI.h>
#include <rendering/VBOCache.h>
#include <world/Direction.h>

#include <entity/Player.h>

#include <stdbool.h>
#include <vec/vec.h>

#include <3ds.h>

const Vertex cube_sides_lut[] = {
    // Fourth face (MX)
    // First triangle
    {{0, 0, 0}, {0, 0, 255}},
    {{0, 0, 1}, {1, 0, 255}},
    {{0, 1, 1}, {1, 1, 255}},
    // Second triangle
    {{0, 1, 1}, {1, 1, 255}},
    {{0, 1, 0}, {0, 1, 255}},
    {{0, 0, 0}, {0, 0, 255}},
    // Third face (PX)
    // First triangle
    {{1, 0, 0}, {1, 0, 255}},
    {{1, 1, 0}, {1, 1, 255}},
    {{1, 1, 1}, {0, 1, 255}},
    // Second triangle
    {{1, 1, 1}, {0, 1, 255}},
    {{1, 0, 1}, {0, 0, 255}},
    {{1, 0, 0}, {1, 0, 255}},
    // Sixth face (MY)
    // First triangle
    {{0, 0, 0}, {0, 1, 255}},
    {{1, 0, 0}, {1, 1, 255}},
    {{1, 0, 1}, {1, 0, 255}},
    // Second triangle
    {{1, 0, 1}, {1, 0, 255}},
    {{0, 0, 1}, {0, 0, 255}},
    {{0, 0, 0}, {0, 1, 255}},
    // Fifth face (PY)
    // First triangle
    {{0, 1, 0}, {0, 1, 255}},
    {{0, 1, 1}, {0, 0, 255}},
    {{1, 1, 1}, {1, 0, 255}},
    // Second triangle
    {{1, 1, 1}, {1, 0, 255}},
    {{1, 1, 0}, {1, 1, 255}},
    {{0, 1, 0}, {0, 1, 255}},
    // Second face (MZ)
    // First triangle
    {{0, 0, 0}, {1, 0, 255}},
    {{0, 1, 0}, {1, 1, 255}},
    {{1, 1, 0}, {0, 1, 255}},
    // Second triangle
    {{1, 1, 0}, {0, 1, 255}},
    {{1, 0, 0}, {0, 0, 255}},
    {{0, 0, 0}, {1, 0, 255}},
    // First face (PZ)
    // First triangle
    {{0, 0, 1}, {0, 0, 255}},
    {{1, 0, 1}, {1, 0, 255}},
    {{1, 1, 1}, {1, 1, 255}},
    // Second triangle
    {{1, 1, 1}, {1, 1, 255}},
    {{0, 1, 1}, {0, 1, 255}},
    {{0, 0, 1}, {0, 0, 255}},
};

typedef struct {
	VBO_Block vbo;
	int x, y, z;
	size_t vertices;
	uint8_t delay;
	uint16_t visibility;
} VBOUpdate;

static vec_t(VBOUpdate) vboUpdates;

#define MAX_FACES_PER_CLUSTER (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2 * 6)

typedef struct {
	int8_t x, y, z;
	Direction direction;
	Block block;
	int8_t ao;
} Face;

static inline Block fastBlockFetch(World* world, Chunk* chunk, Cluster* cluster, int x, int y, int z) {
	return (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE)
		   ? World_GetBlock(world, (chunk->x * CHUNK_SIZE) + x, (cluster->y * CHUNK_SIZE) + y, (chunk->z * CHUNK_SIZE) + z)
		   : cluster->blocks[x][y][z];
}

static World* world;
static Player* player;

typedef struct { int8_t x, y, z; } QueueElement;

static vec_t(QueueElement) floodfill_queue;

static LightLock updateLock;

void PolyGen_Init(World* world_, Player* player_) {
	world = world_;
	player = player_;

	VBOCache_Init();

	vec_init(&floodfill_queue);

	LightLock_Init(&updateLock);

	vec_init(&vboUpdates);
}
void PolyGen_Deinit() {
	vec_deinit(&vboUpdates);

	VBOCache_Deinit();

	vec_deinit(&floodfill_queue);
}

void PolyGen_Harvest() {
	if (LightLock_TryLock(&updateLock) == 0) {
		DebugUI_Text("VBOUpdates %d", vboUpdates.length);
		if (vboUpdates.length > 0) {
			if (vboUpdates.data[0].delay++ > 2)
				while (vboUpdates.length > 0) {
					VBOUpdate update = vec_pop(&vboUpdates);

					Chunk* chunk = World_GetChunk(world, update.x, update.z);
					if (chunk) {
						VBO_Free(chunk->clusters[update.y].vbo);
						chunk->clusters[update.y].vbo = update.vbo;
						chunk->clusters[update.y].vertices = update.vertices;
						chunk->clusters[update.y].seeThrough = update.visibility;
					}
				}
		}

		LightLock_Unlock(&updateLock);
	}
}

static Face faceBuffer[MAX_FACES_PER_CLUSTER];
static int currentFace;
static uint8_t floodfill_visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

static inline void addFace(int x, int y, int z, Direction dir, Block block, int ao) {
	/*for (int i = 0; i < currentFace; i++)
		if (faceBuffer[i].x == x && faceBuffer[i].y == y && faceBuffer[i].z == z && faceBuffer[i].direction == dir) return;
*/ if (x >= 0 && y >= 0 && z >= 0 && x < CHUNK_SIZE && y < CHUNK_SIZE && z < CHUNK_SIZE)
		faceBuffer[currentFace++] = (Face){x, y, z, dir, block, ao};
}

static uint16_t floodFill(Cluster* cluster, int x, int y, int z, Direction entrySide0, Direction entrySide1, Direction entrySide2) {
	if (floodfill_visited[x][y][z] & 1) return 0;
	uint8_t exitPoints[6] = {false};
	if (entrySide0 != Direction_Invalid) exitPoints[entrySide0] = true;
	if (entrySide1 != Direction_Invalid) exitPoints[entrySide1] = true;
	if (entrySide2 != Direction_Invalid) exitPoints[entrySide2] = true;
	vec_clear(&floodfill_queue);
	vec_push(&floodfill_queue, ((QueueElement){x, y, z}));

	while (floodfill_queue.length > 0) {
		QueueElement item = vec_pop(&floodfill_queue);

		for (int i = 0; i < 6; i++) {
			const int* offset = DirectionToOffset[i];
			int x = item.x + offset[0], y = item.y + offset[1], z = item.z + offset[2];
			if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) {
				exitPoints[i] = true;
			} else if (cluster->blocks[x][y][z] == Block_Air) {
				if (!(floodfill_visited[x][y][z] & 1)) {
					floodfill_visited[item.x][item.y][item.z] |= 1;
					vec_push(&floodfill_queue, ((QueueElement){x, y, z}));
				}
			} else if (!(floodfill_visited[item.x][item.y][item.z] & 2) && cluster->blocks[x][y][z] != Block_Air) {
				addFace(x, y, z, DirectionOpposite[i], cluster->blocks[x][y][z], 0);
			}
		}
		floodfill_visited[item.x][item.y][item.z] |= 2;
	}
	uint16_t visiblity = 0;
	for (int i = 0; i < 6; i++)
		if (exitPoints[i])
			for (int j = 0; j < 6; j++)
				if (i != j && exitPoints[j]) visiblity |= ChunkSeeThrough(i, j);
	return visiblity;
}

void PolyGen_GeneratePolygons(WorkQueue* queue, WorkerItem item, void* this) {
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		Cluster* cluster = &item.chunk->clusters[i];

		if (cluster->revision != cluster->vboRevision || cluster->forceVBOUpdate) {
			cluster->vboRevision = cluster->revision;
			cluster->forceVBOUpdate = false;

			currentFace = 0;

			uint16_t visiblity = 0;

			memset(floodfill_visited, 0, sizeof(floodfill_visited));
			for (int x = 0; x < CHUNK_SIZE; x += CHUNK_SIZE - 1) {
				Direction xDir = !x ? Direction_West : Direction_East;
				for (int z = 0; z < CHUNK_SIZE; z++) {
					Direction zDir = Direction_Invalid;
					if (z == 0)
						zDir = Direction_North;
					else if (z == CHUNK_SIZE - 1)
						zDir = Direction_South;
					for (int y = 0; y < CHUNK_SIZE; y++) {
						Direction yDir = Direction_Invalid;
						if (y == 0)
							yDir = Direction_Bottom;
						else if (y == CHUNK_SIZE - 1)
							yDir = Direction_Top;
						if (cluster->blocks[x][y][z] == Block_Air) {
							visiblity |= floodFill(cluster, x, y, z, xDir, yDir, zDir);
						} else if (fastBlockFetch(world, item.chunk, cluster, x + (!x ? -1 : 1), y, z) == Block_Air) {
							addFace(x, y, z, xDir, cluster->blocks[x][y][z], 0);
						}
					}
				}
			}
			for (int y = 0; y < CHUNK_SIZE; y += CHUNK_SIZE - 1) {
				Direction yDir = !y ? Direction_Bottom : Direction_Top;
				for (int x = 0; x < CHUNK_SIZE; x++) {
					Direction xDir = Direction_Invalid;
					if (x == 0)
						xDir = Direction_West;
					else if (x == CHUNK_SIZE - 1)
						xDir = Direction_East;
					for (int z = 0; z < CHUNK_SIZE; z++) {
						Direction zDir = Direction_Invalid;
						if (z == 0)
							zDir = Direction_South;
						else if (z == CHUNK_SIZE - 1)
							zDir = Direction_North;
						if (cluster->blocks[x][y][z] == Block_Air) {
							visiblity |= floodFill(cluster, x, y, z, xDir, yDir, zDir);
						} else if (fastBlockFetch(world, item.chunk, cluster, x, y + (!y ? -1 : 1), z) == Block_Air) {
							addFace(x, y, z, yDir, cluster->blocks[x][y][z], 0);
						}
					}
				}
			}
			for (int z = 0; z < CHUNK_SIZE; z += CHUNK_SIZE - 1) {
				Direction zDir = !z ? Direction_North : Direction_South;
				for (int x = 0; x < CHUNK_SIZE; x++) {
					Direction xDir = Direction_Invalid;
					if (x == 0)
						xDir = Direction_West;
					else if (x == CHUNK_SIZE - 1)
						xDir = Direction_East;
					for (int y = 0; y < CHUNK_SIZE; y++) {
						Direction yDir = Direction_Invalid;
						if (y == 0)
							yDir = Direction_Bottom;
						else if (y == CHUNK_SIZE - 1)
							yDir = Direction_Top;
						if (cluster->blocks[x][y][z] == Block_Air) {
							visiblity |= floodFill(cluster, x, y, z, xDir, yDir, zDir);
						} else if (fastBlockFetch(world, item.chunk, cluster, x, y, z + (!z ? -1 : 1)) == Block_Air) {
							addFace(x, y, z, zDir, cluster->blocks[x][y][z], 0);
						}
					}
				}
			}
			int px = FastFloor(player->position.x);
			int py = FastFloor(player->position.y);
			int pz = FastFloor(player->position.z);
			if (WorldToChunkCoord(px) == item.chunk->x && WorldToChunkCoord(pz) == item.chunk->z && WorldToChunkCoord(py) == i) {
				floodFill(cluster, WorldToLocalCoord(px), WorldToLocalCoord(py), WorldToLocalCoord(pz), Direction_Invalid, Direction_Invalid,
					  Direction_Invalid);
			}

			// if (!currentFace) continue;
			int verticesTotal = currentFace * 6;

			VBO_Block memBlock = VBO_Alloc(verticesTotal * sizeof(Vertex));
			Vertex* data = memBlock.memory;
			for (int j = 0; j < currentFace; j++) {
				Face face = faceBuffer[j];

				int offsetX = face.x + item.chunk->x * CHUNK_SIZE;
				int offsetZ = face.z + item.chunk->z * CHUNK_SIZE;
				int offsetY = face.y + i * CHUNK_SIZE;

				int16_t iconUV[2];
				Block_GetTexture(face.block, face.direction, iconUV);

				memcpy(data, &cube_sides_lut[face.direction * 6], sizeof(Vertex) * 6);

#define oneDivIconsPerRow (32768 / 8)
#define halfTexel (6)

				uint16_t color = (face.block == Block_Grass && face.direction == Direction_Top) ? (uint16_t)((17 << 10) | (26 << 5) | 15)
														: (uint16_t)((31 << 10) | (31 << 5) | 31);

				for (int k = 0; k < 6; k++) {
					data[k].xyz[0] += offsetX;
					data[k].xyz[1] += offsetY;
					data[k].xyz[2] += offsetZ;
					data[k].uvc[0] = (data[k].uvc[0] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[0];
					data[k].uvc[1] = (data[k].uvc[1] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[1];

					data[k].uvc[2] = color;
				}
				data += 6;
			}

			VBOUpdate update;
			update.x = item.chunk->x;
			update.y = i;
			update.z = item.chunk->z;
			update.vbo = memBlock;
			update.vertices = verticesTotal;
			update.delay = 0;
			update.visibility = visiblity;

			LightLock_Lock(&updateLock);
			vec_push(&vboUpdates, update);
			LightLock_Unlock(&updateLock);
		}
	}
	item.chunk->displayRevision = item.chunk->revision;
	item.chunk->forceVBOUpdate = false;
}