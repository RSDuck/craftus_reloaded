#include <world/savegame/SaveManager.h>

#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <mpack/mpack.h>

#include <gui/DebugUI.h>

void SaveManager_Init(SaveManager* mgr, Player* player) {
	mgr->player = player;
	mgr->world = player->world;

	char buffer[256];

#define mkdirFlags S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

	mkdir("sdmc:/craftus", mkdirFlags);
	mkdir("sdmc:/craftus/saves", mkdirFlags);

	sprintf(buffer, "sdmc:/craftus/saves/%s", mgr->world->name);
	mkdir(buffer, mkdirFlags);
	chdir(buffer);

	mkdir("superchunks", mkdirFlags);

	if (access("level.mp", F_OK) != -1) {
		mpack_tree_t levelTree;
		mpack_tree_init_file(&levelTree, "level.mp", 0);
		mpack_node_t root = mpack_tree_root(&levelTree);

		mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, "name"), mgr->world->name, sizeof(mgr->world->name));

		mpack_node_t player = mpack_node_array_at(mpack_node_map_cstr(root, "players"), 0);

		mgr->player->position.x = mpack_node_float(mpack_node_map_cstr(player, "x"));
		mgr->player->position.y = mpack_node_float(mpack_node_map_cstr(player, "y")) + 0.1f;
		mgr->player->position.z = mpack_node_float(mpack_node_map_cstr(player, "z"));

		mgr->player->pitch = mpack_node_float(mpack_node_map_cstr(player, "pitch"));
		mgr->player->yaw = mpack_node_float(mpack_node_map_cstr(player, "yaw"));

		if (mpack_tree_destroy(&levelTree) != mpack_ok) {
			exit(1);
		}
	}

	vec_init(&mgr->superchunks);
}
void SaveManager_Deinit(SaveManager* mgr) {
	mpack_writer_t writer;
	mpack_writer_init_file(&writer, "level.mp");
	mpack_start_map(&writer, 2);

	mpack_write_cstr(&writer, "name");
	mpack_write_cstr(&writer, mgr->world->name);

	mpack_write_cstr(&writer, "players");
	mpack_start_array(&writer, 1);
	mpack_start_map(&writer, 5);

	mpack_write_cstr(&writer, "x");
	mpack_write_float(&writer, mgr->player->position.x);
	mpack_write_cstr(&writer, "y");
	mpack_write_float(&writer, mgr->player->position.y);
	mpack_write_cstr(&writer, "z");
	mpack_write_float(&writer, mgr->player->position.z);

	mpack_write_cstr(&writer, "pitch");
	mpack_write_float(&writer, mgr->player->pitch);
	mpack_write_cstr(&writer, "yaw");
	mpack_write_float(&writer, mgr->player->yaw);

	mpack_finish_map(&writer);
	mpack_finish_array(&writer);

	mpack_finish_map(&writer);

	if (mpack_writer_destroy(&writer) != mpack_ok) {
		exit(1);
	}

	for (int i = 0; i < mgr->superchunks.length; i++) {
		SuperChunk_Deinit(mgr->superchunks.data[i]);
		free(mgr->superchunks.data[i]);
	}

	vec_deinit(&mgr->superchunks);
}

static SuperChunk* fetchSuperChunk(SaveManager* mgr, int x, int z) {
	for (int i = 0; i < mgr->superchunks.length; i++) {
		if (mgr->superchunks.data[i]->x == x && mgr->superchunks.data[i]->z == z) {
			return mgr->superchunks.data[i];
		}
	}
	SuperChunk* superchunk = (SuperChunk*)malloc(sizeof(SuperChunk));
	SuperChunk_Init(superchunk, x, z);
	vec_push(&mgr->superchunks, superchunk);
	return superchunk;
}

void SaveManager_LoadChunk(WorkQueue* queue, WorkerItem item, void* this) {
	SaveManager* mgr = (SaveManager*)this;
	int x = ChunkToSuperChunkCoord(item.chunk->x);
	int z = ChunkToSuperChunkCoord(item.chunk->z);
	SuperChunk* superchunk = fetchSuperChunk(mgr, x, z);

	SuperChunk_LoadChunk(superchunk, item.chunk);
}
void SaveManager_SaveChunk(WorkQueue* queue, WorkerItem item, void* this) {
	SaveManager* mgr = (SaveManager*)this;
	int x = ChunkToSuperChunkCoord(item.chunk->x);
	int z = ChunkToSuperChunkCoord(item.chunk->z);

	SuperChunk* superchunk = fetchSuperChunk(mgr, x, z);

	SuperChunk_SaveChunk(superchunk, item.chunk);
}
