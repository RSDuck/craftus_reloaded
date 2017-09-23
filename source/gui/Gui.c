#include <gui/Gui.h>

#include <stdarg.h>

#include <gui/SpriteBatch.h>
#include <misc/NumberUtils.h>
#include <rendering/VertexFmt.h>

static InputData oldInput;
static InputData input;

typedef struct {
	int width;
	int highestElement;
	int unpaddedWidth;
} Row;

static Row currentRow;
static int relativeX, relativeY;
static int windowX, windowY;
static int paddingX, paddingY;

void Gui_Init() {
	memset(&input, 0x0, sizeof(InputData));
	memset(&oldInput, 0x0, sizeof(InputData));

	paddingX = 2;
	paddingY = 3;
}

static inline int relativeToAbsoluteSize(float s) { return (int)((float)currentRow.unpaddedWidth * s); }

void Gui_Deinit() {}

void Gui_Frame() {
	relativeX = 0;
	relativeY = 0;
	windowX = 0;
	windowY = 0;
}

void Gui_Offset(int x, int y) {
	windowX = x;
	windowY = y;
}

int Gui_RelativeWidth(float x) { return SpriteBatch_GetWidth() * x; }
int Gui_RelativeHeight(float y) { return SpriteBatch_GetHeight() * y; }

void Gui_BeginRowCenter(int width, int count) {
	windowX = SpriteBatch_GetWidth() / 2 - width / 2;
	Gui_BeginRow(width, count);
}

void Gui_BeginRow(int width, int count) {
	currentRow = (Row){width, 0, width - paddingX * 2 - paddingX * count};
	relativeX = paddingX;
	relativeY = 0;
}
void Gui_EndRow() { windowY += currentRow.highestElement + paddingY; }

void Gui_Label(float size, bool shadow, int16_t color, bool center, const char* text, ...) {
	int wrap = size <= 0.f ? (currentRow.width - relativeX - paddingX) : relativeToAbsoluteSize(size);
	int yTextSize = 0;

	va_list vl;
	va_start(vl, text);
	int xOffset = 0;
	if (center) {
		int textWidth = SpriteBatch_CalcTextWidthVargs(text, vl);
		if (textWidth <= wrap) xOffset = wrap / 2 - textWidth / 2;
	}
	int xTextSize =
	    SpriteBatch_PushTextVargs(windowX + relativeX + xOffset, windowY + relativeY, 0, INT16_MAX, false, wrap, &yTextSize, text, vl);
	va_end(vl);
	relativeX += (size <= 0.f ? xTextSize : wrap) + paddingX;
	currentRow.highestElement = MAX(currentRow.highestElement, yTextSize);
}

bool Gui_Button(float size, const char* label, ...) {
#define SLICE_SIZE 8

	va_list vl;
	va_start(vl, label);

	int textWidth = SpriteBatch_CalcTextWidthVargs(label, vl);

	int x = windowX + relativeX;
	int y = windowY + relativeY - BUTTON_TEXT_PADDING;
	int w = (size <= 0.f) ? textWidth + SLICE_SIZE : relativeToAbsoluteSize(size);

	bool pressed = Gui_IsCursorInside(x, y, w, BUTTON_HEIGHT);

	int middlePieceSize = w - SLICE_SIZE * 2;

	SpriteBatch_BindGuiTexture(GuiTexture_Widgets);
	SpriteBatch_PushQuad(x, y, -3, SLICE_SIZE, 20, 0, 46 + (pressed * BUTTON_HEIGHT * 2), SLICE_SIZE, 20);
	SpriteBatch_PushQuad(x + SLICE_SIZE, y, -3, middlePieceSize, 20, SLICE_SIZE, 46 + (pressed * BUTTON_HEIGHT * 2), middlePieceSize,
			     20);
	SpriteBatch_PushQuad(x + SLICE_SIZE + middlePieceSize, y, -3, SLICE_SIZE, 20, 192, 46 + (pressed * BUTTON_HEIGHT * 2), SLICE_SIZE,
			     20);

	SpriteBatch_PushTextVargs(x + (w / 2 - textWidth / 2), y + (BUTTON_HEIGHT - CHAR_HEIGHT) / 2, -1, SHADER_RGB(31, 31, 31), true,
				  INT_MAX, NULL, label, vl);
	va_end(vl);

	relativeX += w + paddingX;
	currentRow.highestElement = MAX(currentRow.highestElement, BUTTON_HEIGHT);

	if (input.keysup & KEY_TOUCH && Gui_WasCursorInside(x, y, w, BUTTON_HEIGHT)) return true;

	return false;
}

void Gui_Space(float space) { relativeX += relativeToAbsoluteSize(space) + paddingX; }
void Gui_VerticalSpace(int y) { windowY += y; }

void Gui_InputData(InputData data) {
	oldInput = input;
	input = data;
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