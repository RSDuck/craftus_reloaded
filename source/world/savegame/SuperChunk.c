#include <world/savegame/SuperChunk.h>

#include <miniz/miniz.h>
#include <mpack/mpack.h>
#include <stdlib.h>
#include <unistd.h>

#include <misc/Crash.h>

static const int SectorSize = 2048;
static mpack_node_data_t* nodeDataPool = NULL;
static const int nodeDataPoolSize = 2048;
const int decompressBufferSize = sizeof(Chunk) * 2;
static char* decompressBuffer;
static const int fileBufferSize = sizeof(Chunk) * 2;
static char* fileBuffer;

void SuperChunk_InitPools() {
	nodeDataPool = (mpack_node_data_t*)malloc(sizeof(mpack_node_data_t) * nodeDataPoolSize);
	fileBuffer = malloc(fileBufferSize);  // 4kb
	decompressBuffer = malloc(decompressBufferSize);
}
void SuperChunk_DeinitPools() {
	free(nodeDataPool);
	free(decompressBuffer);
	free(fileBuffer);
}

void SuperChunk_Init(SuperChunk* superchunk, int x, int z) {
	superchunk->x = x;
	superchunk->z = z;

	vec_init(&superchunk->sectors);

	char buffer[256];
	sprintf(buffer, "superchunks/s.%d.%d.mp", x, z);

	if (access(buffer, F_OK) != -1) {
		int size = 0;
		{
			FILE* f = fopen(buffer, "rb");
			fseek(f, 0, SEEK_END);
			size = ftell(f);
			fseek(f, 0, SEEK_SET);
			fread(fileBuffer, sizeof(char), size, f);
			fclose(f);
		}

		mpack_tree_t tree;
		mpack_tree_init_pool(&tree, fileBuffer, size, nodeDataPool, nodeDataPoolSize);
		mpack_node_t root = mpack_tree_root(&tree);

		mpack_node_t chunkIndices = mpack_node_map_cstr(root, "chunkIndices");
		for (int i = 0; i < SUPERCHUNK_SIZE * SUPERCHUNK_SIZE; i++) {
			mpack_node_t chunkInfo = mpack_node_array_at(chunkIndices, i);
			superchunk->grid[i % SUPERCHUNK_SIZE][i / SUPERCHUNK_SIZE] =
			    (ChunkInfo){mpack_node_u32(mpack_node_map_cstr(chunkInfo, "position")),
					mpack_node_u32(mpack_node_map_cstr(chunkInfo, "compressedSize")),
					mpack_node_u32(mpack_node_map_cstr(chunkInfo, "actualSize")),
					mpack_node_u8(mpack_node_map_cstr(chunkInfo, "blockSize")),
					mpack_node_u32(mpack_node_map_cstr(chunkInfo, "revision"))};

			{
				ChunkInfo chunkInfo = superchunk->grid[i % SUPERCHUNK_SIZE][i / SUPERCHUNK_SIZE];
				if (chunkInfo.actualSize > 0) {
					while (chunkInfo.position + chunkInfo.blockSize > superchunk->sectors.length) {
						vec_push(&superchunk->sectors, false);
					}
					for (int j = 0; j < chunkInfo.blockSize; j++)
						superchunk->sectors.data[chunkInfo.position + j] = true;
				}
			}
		}

		mpack_error_t err = mpack_tree_destroy(&tree);
		if (err != mpack_ok) {
			Crash("MPack error %d while loading superchunk manifest %d %d", err, x, z);
		}
	} else {
		memset(superchunk->grid, 0x0, sizeof(superchunk->grid));
	}

	sprintf(buffer, "superchunks/s.%d.%d.dat", x, z);
	superchunk->dataFile = fopen(buffer, "r+b");
	if (superchunk->dataFile == NULL) superchunk->dataFile = fopen(buffer, "w+b");
}
void SuperChunk_Deinit(SuperChunk* superchunk) {
	SuperChunk_SaveIndex(superchunk);
	vec_deinit(&superchunk->sectors);
	fclose(superchunk->dataFile);
}

void SuperChunk_SaveIndex(SuperChunk* superchunk) {
	char buffer[256];
	sprintf(buffer, "superchunks/s.%d.%d.mp", superchunk->x, superchunk->z);

	mpack_writer_t writer;
	mpack_writer_init_file(&writer, buffer);

	mpack_start_map(&writer, 1);

	mpack_write_cstr(&writer, "chunkIndices");

	mpack_start_array(&writer, SUPERCHUNK_SIZE * SUPERCHUNK_SIZE);
	for (int j = 0; j < SUPERCHUNK_SIZE; j++) {
		for (int i = 0; i < SUPERCHUNK_SIZE; i++) {
			ChunkInfo chunkInfo = superchunk->grid[i][j];
			mpack_start_map(&writer, 5);

			mpack_write_cstr(&writer, "position");
			mpack_write_u32(&writer, chunkInfo.position);
			mpack_write_cstr(&writer, "compressedSize");
			mpack_write_u32(&writer, chunkInfo.compressedSize);
			mpack_write_cstr(&writer, "actualSize");
			mpack_write_u32(&writer, chunkInfo.actualSize);
			mpack_write_cstr(&writer, "blockSize");
			mpack_write_u8(&writer, chunkInfo.blockSize);
			mpack_write_cstr(&writer, "revision");
			mpack_write_u32(&writer, chunkInfo.revision);

			mpack_finish_map(&writer);
		}
	}
	mpack_finish_array(&writer);

	mpack_finish_map(&writer);

	mpack_error_t err = mpack_writer_destroy(&writer);
	if (err != mpack_ok) {
		Crash("Mpack error %d while saving superchunk index %d %d", err, superchunk->x, superchunk->z);
	}
}

static uint32_t reserveSectors(SuperChunk* superchunk, int amount) {
	int amountFulfilled = 0;
	int startValue = -1;
	for (int i = 0; i < superchunk->sectors.length; i++) {
		if (!superchunk->sectors.data[i]) {
			if (startValue == -1) startValue = i;
			amountFulfilled++;
		} else {
			amountFulfilled = 0;
			startValue = -1;
		}
		if (amountFulfilled == amount) {
			for (int i = 0; i < amount; i++) superchunk->sectors.data[startValue + i] = true;
			return startValue;
		}
	}
	for (int i = 0; i < amount; i++) vec_push(&superchunk->sectors, true);
	return superchunk->sectors.length - amount;
}
static void freeSectors(SuperChunk* superchunk, uint32_t address, uint8_t size) {
	for (size_t i = 0; i < size; i++) {
		superchunk->sectors.data[address + i] = false;
	}
}

void SuperChunk_SaveChunk(SuperChunk* superchunk, Chunk* chunk) {
	int x = ChunkToLocalSuperChunkCoord(chunk->x);
	int z = ChunkToLocalSuperChunkCoord(chunk->z);

	if (superchunk->grid[x][z].revision != chunk->revision) {
		mpack_writer_t writer;
		mpack_writer_init(&writer, decompressBuffer, decompressBufferSize);

		mpack_start_map(&writer, 3);

		mpack_write_cstr(&writer, "clusters");
		mpack_start_array(&writer, CLUSTER_PER_CHUNK);
		for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
			bool empty = Cluster_IsEmpty(&chunk->clusters[i]);

			mpack_start_map(&writer, empty ? 2 : 4);

			if (!empty) {
				mpack_write_cstr(&writer, "blocks");
				mpack_write_bin(&writer, (char*)chunk->clusters[i].blocks, sizeof(chunk->clusters[i].blocks));
				mpack_write_cstr(&writer, "metadataLight");
				mpack_write_bin(&writer, (char*)chunk->clusters[i].metadataLight, sizeof(chunk->clusters[i].metadataLight));
			}

			mpack_write_cstr(&writer, "revision");
			mpack_write_u32(&writer, chunk->clusters[i].revision);

			mpack_write_cstr(&writer, "empty");
			mpack_write_bool(&writer, empty);

			mpack_finish_map(&writer);
		}
		mpack_finish_array(&writer);

		mpack_write_cstr(&writer, "genProgress");
		mpack_write_int(&writer, chunk->genProgress);

		mpack_write_cstr(&writer, "heightmap");
		mpack_write_bin(&writer, (char*)chunk->heightmap, sizeof(chunk->heightmap));

		mpack_finish_map(&writer);
		mpack_error_t err = mpack_writer_destroy(&writer);
		if (err != mpack_ok) {
			Crash("MPack error %d while saving chunk(%d, %d) to superchunk", err, chunk->x, chunk->z);
		}

		size_t uncompressedSize = mpack_writer_buffer_used(&writer);
		mz_ulong compressedSize = fileBufferSize;
		if (compress((uint8_t*)fileBuffer, &compressedSize, (uint8_t*)decompressBuffer, uncompressedSize) == Z_OK) {
			size_t blockSize = compressedSize / SectorSize + 1;

			if (superchunk->grid[x][z].actualSize > 0)
				freeSectors(superchunk, superchunk->grid[x][z].position, superchunk->grid[x][z].blockSize);

			size_t address = reserveSectors(superchunk, blockSize);

			fseek(superchunk->dataFile, address * SectorSize, SEEK_SET);
			if (fwrite(fileBuffer, compressedSize, 1, superchunk->dataFile) != 1)
				Crash("Couldn't write complete chunk data to file");

			superchunk->grid[x][z] = (ChunkInfo){address, compressedSize, uncompressedSize, blockSize, chunk->revision};
		}
	}
}

void SuperChunk_LoadChunk(SuperChunk* superchunk, Chunk* chunk) {
	int x = ChunkToLocalSuperChunkCoord(chunk->x);
	int z = ChunkToLocalSuperChunkCoord(chunk->z);
	ChunkInfo chunkInfo = superchunk->grid[x][z];
	if (chunkInfo.actualSize > 0) {
		fseek(superchunk->dataFile, chunkInfo.position * SectorSize, SEEK_SET);
		if (fread(fileBuffer, chunkInfo.compressedSize, 1, superchunk->dataFile) != 1)
			Crash("Read chunk data size isn't equal to the expected size");
		mz_ulong uncompressedSize = decompressBufferSize;

		if (uncompress((uint8_t*)decompressBuffer, &uncompressedSize, (uint8_t*)fileBuffer, chunkInfo.compressedSize) == Z_OK) {
			mpack_tree_t tree;
			mpack_tree_init_pool(&tree, decompressBuffer, uncompressedSize, nodeDataPool, nodeDataPoolSize);
			mpack_node_t root = mpack_tree_root(&tree);

			mpack_node_t clusters = mpack_node_map_cstr(root, "clusters");
			for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
				mpack_node_t cluster = mpack_node_array_at(clusters, i);

				chunk->clusters[i].revision = mpack_node_u32(mpack_node_map_cstr(cluster, "revision"));

				mpack_node_t emptyNode = mpack_node_map_cstr_optional(cluster, "empty");
				if (mpack_node_type(emptyNode) != mpack_type_nil) {
					chunk->clusters[i].emptyRevision = chunk->clusters[i].revision;
					chunk->clusters[i].empty = mpack_node_bool(emptyNode);
				} else {
					chunk->clusters[i].emptyRevision = 0;
					chunk->clusters[i].empty = false;
				}

				mpack_node_t blocksNode = mpack_node_map_cstr_optional(cluster, "blocks");
				if (mpack_node_type(blocksNode) == mpack_type_bin)  // preserve savedata, in case of a wrong empty flag
					memcpy(chunk->clusters[i].blocks, mpack_node_data(blocksNode), sizeof(chunk->clusters[i].blocks));
				mpack_node_t metadataNode = mpack_node_map_cstr_optional(cluster, "metadataLight");
				if (mpack_node_type(metadataNode) == mpack_type_bin)
					memcpy(chunk->clusters[i].metadataLight, mpack_node_data(metadataNode),
					       sizeof(chunk->clusters[i].metadataLight));
			}

			chunk->genProgress = mpack_node_int(mpack_node_map_cstr(root, "genProgress"));

			mpack_node_t heightmapNode = mpack_node_map_cstr(root, "heightmap");
			if (mpack_node_type(heightmapNode) != mpack_type_nil) {
				memcpy(chunk->heightmap, mpack_node_data(heightmapNode), sizeof(chunk->heightmap));
				chunk->heightmapRevision = chunkInfo.revision;
			} else
				chunk->heightmapRevision = 0;

			mpack_error_t err = mpack_tree_destroy(&tree);
			if (err != mpack_ok) {
				Crash("MPack error %d while loading chunk(%d %d) from superchunk", err, chunk->x, chunk->z);
			}

			chunk->revision = chunkInfo.revision;
		}
	}
}