#include <gui/SpriteBatch.h>
#include <rendering/VertexFmt.h>

#include <gui/DebugUI.h>

#include <stdarg.h>
#include <stdlib.h>

#define STATUS_LINES (240 / 8 / 2)
#define LOG_LINES 20
#define LOG_LINE_LENGTH 128
#define STATUS_LINE_LENGTH 128

static char* statusLines[STATUS_LINES];
static char* logLines[LOG_LINES];
static int currentStatusLine = 0;

void DebugUI_Init() {
	for (int i = 0; i < LOG_LINES; i++) {
		logLines[i] = malloc(LOG_LINE_LENGTH);
		memset(logLines[i], 0x0, LOG_LINE_LENGTH);
	}
	for (int i = 0; i < STATUS_LINES; i++) {
		statusLines[i] = malloc(STATUS_LINE_LENGTH);
		memset(statusLines[i], 0x0, STATUS_LINE_LENGTH);
	}
}
void DebugUI_Deinit() {
	for (int i = 0; i < LOG_LINES; i++) free(logLines[i]);
	for (int i = 0; i < STATUS_LINES; i++) free(statusLines[i]);
}

void DebugUI_Text(const char* text, ...) {
	if (currentStatusLine >= STATUS_LINES) return;
	va_list args;
	va_start(args, text);

	vsprintf(statusLines[currentStatusLine++], text, args);

	va_end(args);
}

void DebugUI_Log(const char* text, ...) {
	char* lastLine = logLines[LOG_LINES - 1];
	for (int i = LOG_LINES - 1; i > 0; i--) logLines[i] = logLines[i - 1];
	logLines[0] = lastLine;

	va_list args;
	va_start(args, text);

	vsprintf(logLines[0], text, args);

	va_end(args);
}

void DebugUI_Draw() {
	SpriteBatch_SetScale(1);

	//SpriteBatch_PushSingleColorQuad(0, 0, 1, 320, 240, SHADER_RGB(4, 4, 4));

	int yOffset = (240 / 3) * 2;
	for (int i = 0; i < LOG_LINES; i++) {
		int step = 0;
		SpriteBatch_PushText(0, yOffset, 100, INT16_MAX, false, 320, &step, "%s", logLines[i]);
		yOffset += step;
		if (yOffset >= 240) break;
	}
	yOffset = 0;
	for (int i = 0; i < STATUS_LINES; i++) {
		int step = 0;
		SpriteBatch_PushText(0, yOffset, 100, INT16_MAX, false, 320, &step, "%s", statusLines[i]);
		yOffset += step;

		memset(statusLines[i], 0x0, STATUS_LINE_LENGTH);
	}
	currentStatusLine = 0;
}