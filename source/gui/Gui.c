#include <gui/Gui.h>

#include <gui/SpriteBatch.h>
#include <rendering/VertexFmt.h>

static InputData oldInput;
static InputData input;

void Gui_Init() {
	memset(&input, 0x0, sizeof(InputData));
	memset(&oldInput, 0x0, sizeof(InputData));
}

void Gui_Deinit() {}

void Gui_InputData(InputData data) {
	oldInput = input;
	input = data;
}

bool Gui_Button(int x, int y, int w, const char* text) {
#define SLICE_SIZE 8

	int textWidth = SpriteBatch_CalcTextWidth(text);

	if (w == -1) w = textWidth + SLICE_SIZE;

	bool pressed = Gui_IsCursorInside(x, y, w, BUTTON_HEIGHT);

	int middlePieceSize = w - SLICE_SIZE * 2;

	SpriteBatch_BindGuiTexture(GuiTexture_Widgets);
	SpriteBatch_PushQuad(x, y, -2, SLICE_SIZE, 20, 0, 46 + (pressed * BUTTON_HEIGHT * 2), SLICE_SIZE, 20);
	SpriteBatch_PushQuad(x + SLICE_SIZE, y, -2, middlePieceSize, 20, SLICE_SIZE, 46 + (pressed * BUTTON_HEIGHT * 2), middlePieceSize, 20);
	SpriteBatch_PushQuad(x + SLICE_SIZE + middlePieceSize, y, -2, SLICE_SIZE, 20, 192, 46 + (pressed * BUTTON_HEIGHT * 2), SLICE_SIZE, 20);

	SpriteBatch_PushText(x + (w / 2 - textWidth / 2), y + (BUTTON_HEIGHT - CHAR_HEIGHT) / 2, 0, SHADER_RGB(31, 31, 31), true, INT_MAX, NULL,
		     text);

	if (input.keysup & KEY_TOUCH && Gui_WasCursorInside(x, y, w, BUTTON_HEIGHT)) return true;

	return false;
}

bool Gui_IsCursorInside(int x, int y, int w, int h) {
	int sclInputX = input.touchX / SpriteBatch_GetScale();
	int sclInputY = input.touchY / SpriteBatch_GetScale();
	return sclInputX != 0 && sclInputY != 0 && sclInputX >= x && sclInputX < x + w && sclInputY >= y && sclInputY < y + h;
}
bool Gui_WasCursorInside(int x, int y, int w, int h) {
	int sclOldInputX = oldInput.touchX / SpriteBatch_GetScale();
	int sclOldInputY = oldInput.touchY / SpriteBatch_GetScale();
	return sclOldInputX != 0 && sclOldInputY != 0 && sclOldInputX >= x && sclOldInputX < x + w && sclOldInputY >= y &&
	       sclOldInputY < y + h;
}
bool Gui_EnteredCursorInside(int x, int y, int w, int h) {
	int sclOldInputX = oldInput.touchX / SpriteBatch_GetScale();
	int sclOldInputY = oldInput.touchY / SpriteBatch_GetScale();

	return (sclOldInputX == 0 && sclOldInputY == 0) && Gui_IsCursorInside(x, y, w, h);
}
void Gui_GetCursorMovement(int* x, int* y) {
	if ((input.touchX == 0 && input.touchY == 0) || (oldInput.touchX == 0 && oldInput.touchY == 0)) {
		*x = 0;
		*y = 0;
		return;
	}
	*x = input.touchX / SpriteBatch_GetScale() - oldInput.touchX / SpriteBatch_GetScale();
	*y = input.touchY / SpriteBatch_GetScale() - oldInput.touchY / SpriteBatch_GetScale();
}