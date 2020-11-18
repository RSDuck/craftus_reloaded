#include <gui/WorldSelect.h>

#include <gui/Gui.h>
#include <gui/SpriteBatch.h>

#include <vec/vec.h>

#include <rendering/VertexFmt.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mpack/mpack.h>

#include <3ds.h>

typedef struct {
	uint32_t lastPlayed;
	char name[WORLD_NAME_SIZE];
	char path[256];
} WorldInfo;

static vec_t(WorldInfo) worlds;

void WorldSelect_ScanWorlds() {
	vec_clear(&worlds);

	DIR* directory = opendir("sdmc:/craftus/saves");

	char buffer[512];

	struct dirent* entry;

	while ((entry = readdir(directory))) {
		sprintf(buffer, "sdmc:/craftus/saves/%s/level.mp", entry->d_name);
		if (access(buffer, F_OK) != -1) {
			mpack_tree_t tree;
			mpack_tree_init_file(&tree, buffer, 0);
			mpack_node_t root = mpack_tree_root(&tree);

			char name[WORLD_NAME_SIZE];
			mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, "name"), name, WORLD_NAME_SIZE);

			if (mpack_tree_destroy(&tree) != mpack_ok) {
				continue;
			}

			WorldInfo info;
			strcpy(info.name, name);
			info.lastPlayed = 0;
			strcpy(info.path, entry->d_name);

			vec_push(&worlds, info);
		}
	}

	closedir(directory);
}

static void delete_folder(const char* path) {
	DIR* dir = opendir(path);
	struct dirent* entry;

	int pathLen = strlen(path);

	while ((entry = readdir(dir))) {
		if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) continue;

		int entryLen = strlen(entry->d_name);

		char buffer[pathLen + entryLen + 1];
		sprintf(buffer, "%s/%s", path, entry->d_name);

		if (entry->d_type == DT_DIR)
			delete_folder(buffer);
		else
			unlink(buffer);
	}

	closedir(dir);

	rmdir(path);
}

void WorldSelect_Init() {
	vec_init(&worlds);

	WorldSelect_ScanWorlds();
}

void WorldSelect_Deinit() { vec_deinit(&worlds); }

typedef enum { MenuState_SelectWorld, MenuState_ConfirmDeletion, MenuState_WorldOptions } MenuState;

static int scroll = 0;
static float velocity = 0.f;
static int selectedWorld = -1;

static bool clicked_play = false;
static bool clicked_new_world = false;
static bool clicked_delete_world = false;

static bool confirmed_world_options = false;
static bool canceled_world_options = false;

static bool confirmed_deletion = false;
static bool canceled_deletion = false;

static WorldGenType worldGenType = WorldGen_SuperFlat;

static char* worldGenTypesStr[] = {"Smea", "Superflat"};

static MenuState menustate = MenuState_SelectWorld;

static float max_velocity = 20.f;

void WorldSelect_Render() {
	SpriteBatch_SetScale(2);

	SpriteBatch_BindGuiTexture(GuiTexture_MenuBackground);
	for (int i = 0; i < 160 / 32 + 1; i++) {
		for (int j = 0; j < 120 / 32 + 1; j++) {
			bool overlay = j >= 2 && menustate == MenuState_SelectWorld;
			SpriteBatch_PushQuadColor(i * 32, j * 32, overlay ? -4 : -10, 32, 32, 0, 0, 32, 32,
						  overlay ? INT16_MAX : SHADER_RGB(12, 12, 12));
		}
	}

	if (menustate == MenuState_SelectWorld) {
		int movementX = 0, movementY = 0;
		Gui_GetCursorMovement(&movementX, &movementY);
		if (Gui_IsCursorInside(0, 0, 160, 2 * 32)) {
			velocity += movementY / 2.f;
			velocity = CLAMP(velocity, -max_velocity, max_velocity);
		}
		scroll += velocity;
		velocity *= 0.75f;
		if (ABS(velocity) < 0.001f) velocity = 0.f;

		int maximumSize = CHAR_HEIGHT * 2 * worlds.length;
		if (scroll < -maximumSize) scroll = -maximumSize;
		if (scroll > 0) scroll = 0;

		WorldInfo info;
		int i = 0;
		vec_foreach (&worlds, info, i) {
			int y = i * (CHAR_HEIGHT + CHAR_HEIGHT) + 10 + scroll;
			if (selectedWorld == i) {
				SpriteBatch_PushSingleColorQuad(10, y - 3, -7, 140, 1, SHADER_RGB(20, 20, 20));
				SpriteBatch_PushSingleColorQuad(10, y + CHAR_HEIGHT + 2, -7, 140, 1, SHADER_RGB(20, 20, 20));
				SpriteBatch_PushSingleColorQuad(10, y - 3, -7, 1, CHAR_HEIGHT + 6, SHADER_RGB(20, 20, 20));
				SpriteBatch_PushSingleColorQuad(10 + 140, y - 3, -7, 1, CHAR_HEIGHT + 6, SHADER_RGB(20, 20, 20));
			}
			if (Gui_EnteredCursorInside(10, y - 3, 140, CHAR_HEIGHT + 6) && y < 32 * 2) {
				selectedWorld = i;
			}
			SpriteBatch_PushText(20, y, -6, INT16_MAX, true, INT_MAX, NULL, "%s", info.name, movementY);
		}

		Gui_Offset(0, 2 * 32 + 5 + BUTTON_TEXT_PADDING);
		Gui_BeginRowCenter(Gui_RelativeWidth(0.95f), 1);
		clicked_play = Gui_Button(1.f, "Play selected world");
		Gui_EndRow();
		Gui_BeginRowCenter(Gui_RelativeWidth(0.95f), 2);
		clicked_new_world = Gui_Button(0.5f, "New World");
		clicked_delete_world = Gui_Button(0.5f, "Delete World");
		Gui_EndRow();
	} else if (menustate == MenuState_ConfirmDeletion) {
		Gui_Offset(0, 10);
		Gui_BeginRow(SpriteBatch_GetWidth(), 1);
		Gui_Label(0.f, true, INT16_MAX, true, "Are you sure?");
		Gui_EndRow();
		Gui_VerticalSpace(Gui_RelativeHeight(0.4f));
		Gui_BeginRowCenter(Gui_RelativeWidth(0.8f), 3);
		canceled_deletion = Gui_Button(0.4f, "No");
		Gui_Space(0.2f);
		confirmed_deletion = Gui_Button(0.4f, "Yes");
		Gui_EndRow();
	} else if (menustate == MenuState_WorldOptions) {
		Gui_Offset(0, 10);
		Gui_BeginRowCenter(Gui_RelativeWidth(0.9f), 3);
		Gui_Label(0.45f, true, INT16_MAX, false, "World type:");
		Gui_Space(0.1f);
		if (Gui_Button(0.45f, "%s", worldGenTypesStr[worldGenType])) {
			worldGenType++;
			if (worldGenType == WorldGenTypes_Count) worldGenType = 0;
		}
		Gui_EndRow();

		Gui_VerticalSpace(Gui_RelativeHeight(0.4f));

		Gui_BeginRowCenter(Gui_RelativeWidth(0.9f), 3);
		canceled_world_options = Gui_Button(0.45f, "Cancel");
		Gui_Space(0.1f);
		confirmed_world_options = Gui_Button(0.45f, "Continue");
	}
}

bool WorldSelect_Update(char* out_worldpath, char* out_name, WorldGenType* worldType, bool* newWorld) {
	if (clicked_new_world) {
		clicked_new_world = false;
		menustate = MenuState_WorldOptions;
	}
	if (confirmed_world_options) {
		confirmed_world_options = false;
		*worldType = worldGenType;

		static SwkbdState swkbd;
		static char name[WORLD_NAME_SIZE];

#ifndef _DEBUG
		swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, WORLD_NAME_SIZE);
		swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, WORLD_NAME_SIZE);
		swkbdSetHintText(&swkbd, "Enter the world name");

		int button = swkbdInputText(&swkbd, name, 12);
#else
		strcpy(name, "testworld");
		int button = 2;
#endif

		strcpy(out_name, name);
		menustate = MenuState_SelectWorld;
		if (button == SWKBD_BUTTON_CONFIRM) {
			strcpy(out_worldpath, out_name);

			int length = strlen(out_worldpath);

			for (int i = 0; i < length; i++) {
				if (out_worldpath[i] == '/' || out_worldpath[i] == '\\' || out_worldpath[i] == '?' ||
				    out_worldpath[i] == ':' || out_worldpath[i] == '|' || out_worldpath[i] == '<' ||
				    out_worldpath[i] == '>')
					out_worldpath[i] = '_';
			}

			while (true) {
				int i;
				WorldInfo* info;
				bool alreadyExisting = false;
				vec_foreach_ptr(&worlds, info, i) if (!strcmp(out_worldpath, info->path)) {
					alreadyExisting = true;
					break;
				}
				if (!alreadyExisting) break;

				out_worldpath[length] = '_';
				out_worldpath[length + 1] = '\0';
				++length;
			}

			*newWorld = true;

			return true;
		}
	}
	if (clicked_play && selectedWorld != -1) {
		clicked_play = false;
		strcpy(out_name, worlds.data[selectedWorld].name);
		strcpy(out_worldpath, worlds.data[selectedWorld].path);

		*newWorld = false;
		menustate = MenuState_SelectWorld;
		return true;
	}
	if (clicked_delete_world && selectedWorld != -1) {
		clicked_delete_world = false;
		menustate = MenuState_ConfirmDeletion;
	}
	if (confirmed_deletion) {
		confirmed_deletion = false;
		char buffer[512];
		sprintf(buffer, "sdmc:/craftus/saves/%s", worlds.data[selectedWorld].path);
		delete_folder(buffer);

		WorldSelect_ScanWorlds();
		menustate = MenuState_SelectWorld;
	}
	if (canceled_deletion) {
		canceled_deletion = false;
		menustate = MenuState_SelectWorld;
	}
	if (canceled_world_options) {
		canceled_world_options = false;
		menustate = MenuState_SelectWorld;
	}

	return false;
}