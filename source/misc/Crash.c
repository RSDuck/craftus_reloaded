#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <3ds.h>

#include <misc/Crash.h>

void Crash(const char* reason, ...) {
	consoleInit(GFX_TOP, NULL);

	va_list vl;
	va_start(vl, reason);
	vprintf(reason, vl);

	FILE* f = fopen("sdmc:/craftus/crash.txt", "w");
	vfprintf(f, reason, vl);
	fclose(f);

	va_end(vl);

	printf("\n\nFatal error, press start to exit\n");
	while (aptMainLoop()) {
		gspWaitForVBlank();

		hidScanInput();

		if (hidKeysDown() & KEY_START) break;
	}

	exit(EXIT_FAILURE);
}
