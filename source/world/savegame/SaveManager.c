#include <world/savegame/SaveManager.h>

#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <mpack/mpack.h>

#include <gui/DebugUI.h>
#include <misc/Crash.h>

#define mkdirFlags S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

void SaveManager_InitFileSystem() {
	mkdir("sdmc:/craftus", mkdirFlags);
	mkdir("sdmc:/craftus/saves", mkdirFlags);
}

void SaveManager_Init(SaveManager* mgr, Player* player) {
	mgr->player = player;
	mgr->world = player->world;

	vec_init(&mgr->superchunks);
}
void SaveManager_Deinit(SaveManager* mgr) { vec_deinit(&mgr->superchunks); }

#define mpack_elvis(node, key, typ, default_) \
	((mpack_node_type(mpack_node_map_cstr_optional((node), (key))) != mpack_type_nil) ? mpack_node_ ## typ (mpack_node_map_cstr_optional((node), (key))) : (default_))
	

void SaveManager_Load(SaveManager* mgr, char* path) {
	char buffer[256];

	sprintf(buffer, "sdmc:/craftus/saves/%s", path);
	mkdir(buffer, mkdirFlags);
	chdir(buffer);

	mkdir("superchunks", mkdirFlags);

	if (access("level.mp", F_OK) != -1) {
		mpack_tree_t levelTree;
		mpack_tree_init_file(&levelTree, "level.mp", 0);
		mpack_node_t root = mpack_tree_root(&levelTree);

		mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, "name"), mgr->world->name, sizeof(mgr->world->name));

		mpack_node_t worldTypeNode = mpack_node_map_cstr_optional(root, "worldType");
		if (mpack_node_type(worldTypeNode) != mpack_type_nil)
			mgr->world->genSettings.type = mpack_node_uint(worldTypeNode);
		else
			mgr->world->genSettings.type = WorldGen_SuperFlat;

		mpack_node_t player = mpack_node_array_at(mpack_node_map_cstr(root, "players"), 0);

		mgr->player->position.x = mpack_node_float(mpack_node_map_cstr(player, "x"));
		mgr->player->position.y = mpack_node_float(mpack_node_map_cstr(player, "y")) + 0.1f;
		mgr->player->position.z = mpack_node_float(mpack_node_map_cstr(player, "z"));

		mgr->player->pitch = mpack_node_float(mpack_node_map_cstr(player, "pitch"));
		mgr->player->yaw = mpack_node_float(mpack_node_map_cstr(player, "yaw"));

		mgr->player->flying = mpack_elvis(player, "flying", bool, false);
		mgr->player->crouching = mpack_elvis(player, "crouching", bool, false);

		mpack_error_t err = mpack_tree_destroy(&levelTree);
		if (err != mpack_ok) {
			Crash("Mpack error %d while loading world manifest %s", err, path);
		}
	}
}

void SaveManager_Unload(SaveManager* mgr) {
	mpack_writer_t writer;
	mpack_writer_init_file(&writer, "level.mp");
	mpack_start_map(&writer, 3);

	mpack_write_cstr(&writer, "name");
	mpack_write_cstr(&writer, mgr->world->name);

	mpack_write_cstr(&writer, "players");
	mpack_start_array(&writer, 1);
	mpack_start_map(&writer, 7);

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

	mpack_write_cstr(&writer, "flying");
	mpack_write_bool(&writer, mgr->player->flying);
	
	mpack_write_cstr(&writer, "crouching");
	mpack_write_bool(&writer, mgr->player->crouching);

	mpack_finish_map(&writer);
	mpack_finish_array(&writer);

	mpack_write_cstr(&writer, "worldType");
	mpack_write_uint(&writer, mgr->world->genSettings.type);

	mpack_finish_map(&writer);

	mpack_error_t err = mpack_writer_destroy(&writer);
	if (err != mpack_ok) {
		Crash("Mpack error %d while saving world manifest", err);
	}

	for (int i = 0; i < mgr->superchunks.length; i++) {
		SuperChunk_Deinit(mgr->superchunks.data[i]);
		free(mgr->superchunks.data[i]);
	}
	vec_clear(&mgr->superchunks);
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
	svcSleepThread(50000);
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
