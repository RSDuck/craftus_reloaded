#include <misc/CommandLine.h>

#include <3ds.h>

#include <gui/DebugUI.h>

#include <stdio.h>
#include <string.h>

void CommandLine_Activate(World* world, Player* player) {
	static SwkbdState swkbd;
	static char textBuffer[64];
	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, 64);
	swkbdSetHintText(&swkbd, "Enter command");

	int button = swkbdInputText(&swkbd, textBuffer, sizeof(textBuffer));
	if (button == SWKBD_BUTTON_CONFIRM) {
		CommandLine_Execute(world, player, textBuffer);
	}
}

void CommandLine_Execute(World* world, Player* player, const char* text) {
	int length = strlen(text);
	if (length >= 1 && text[0] == '/') {
		if (length >= 9) {
			float x, y, z;
			if (sscanf(&text[1], "tp %f %f %f", &x, &y, &z) == 3) {
				player->position.x = x;
				player->position.y = y;
				player->position.z = z;
				DebugUI_Log("teleported to %f, %f %f", x, y, z);
				return;
			}
		}
		if (length == 2 && text[1] == 'd') {
			extern bool showDebugInfo;
			showDebugInfo ^= true;
		}
	}
}