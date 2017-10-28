#include <gui/Inventory.h>

#include <gui/Gui.h>
#include <gui/SpriteBatch.h>
#include <rendering/VertexFmt.h>

static ItemStack *sourceStack = NULL, *proposedSourceStack = NULL;

static void clickAtStack(ItemStack* stack) {
	if (sourceStack == NULL && stack != proposedSourceStack) {
		proposedSourceStack = stack;
	} else if (proposedSourceStack == stack) {
		sourceStack = stack;
		proposedSourceStack = NULL;
	} else if (sourceStack != NULL) {
		if (sourceStack != stack) ItemStack_Transfer(sourceStack, stack);
		sourceStack = NULL;
	}
}

void Inventory_DrawQuickSelect(int x, int y, ItemStack* stacks, int count, int* selected) {
	SpriteBatch_BindGuiTexture(GuiTexture_Widgets);

	for (int i = 0; i < count; i++) {
		SpriteBatch_SetScale(1);  // TODO: muss verbessert werden für Ports
		int rx = (i * 20 + x + 3) * 2;
		int ry = (y + 3) * 2;
		if (stacks[i].amount > 0) SpriteBatch_PushIcon(stacks[i].block, stacks[i].meta, rx, ry, 11);
		if (Gui_EnteredCursorInside(rx - 4, ry - 4, 18 * 2, 18 * 2)) {
			*selected = i;
			clickAtStack(&stacks[i]);
		}
		SpriteBatch_SetScale(2);
		if (sourceStack == &stacks[i]) {
			SpriteBatch_PushSingleColorQuad(rx / 2 - 2, ry / 2 - 2, 9, 18, 18, SHADER_RGB(20, 5, 2));
			SpriteBatch_BindGuiTexture(GuiTexture_Widgets);
		}
		if (i < count - 2) {
			SpriteBatch_PushQuad(i * 20 + 21 + x, y, 10, 20, 22, 21, 0, 20, 22);
		}
	}
	SpriteBatch_SetScale(2);

	SpriteBatch_PushQuad(x, y, 10, 21, 22, 0, 0, 21, 22);
	SpriteBatch_PushQuad(21 + 20 * (count - 2) + x, y, 10, 21, 22, 161, 0, 21, 22);

	SpriteBatch_PushQuad(x + *selected * 20 - 1, y - 1, 14, 24, 24, 0, 22, 24, 24);
}

void Inventory_Draw(int x, int y, int w, ItemStack* stacks, int count) {
	SpriteBatch_SetScale(1);

	int headX = x;
	int headY = y;
	bool even = false;

	const int16_t colors[2] = {SHADER_RGB_DARKEN(SHADER_RGB(20, 20, 21), 9), SHADER_RGB_DARKEN(SHADER_RGB(20, 20, 21), 8)};

	for (int i = 0; i < count; i++) {
		if (stacks[i].amount > 0) SpriteBatch_PushIcon(stacks[i].block, stacks[i].meta, headX * 2, headY * 2, 10);
		if (Gui_EnteredCursorInside(headX * 2, headY * 2, 16 * 2, 16 * 2)) clickAtStack(&stacks[i]);
		SpriteBatch_PushSingleColorQuad(headX * 2, headY * 2, 9, 16 * 2, 16 * 2,
						sourceStack == &stacks[i] ? SHADER_RGB(20, 5, 2) : colors[even]);
		even ^= true;
		headX += 16;
		if (headX >= w) {
			headX = x;
			headY += 17;
			even = false;
			SpriteBatch_PushSingleColorQuad(x * 2, (headY - 1) * 2, 10, w * 2, 2, SHADER_RGB(7, 7, 7));
		}
	}

	SpriteBatch_SetScale(2);
}