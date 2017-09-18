#pragma once

#include <stdbool.h>

#include <misc/InputData.h>

void Gui_Init();
void Gui_Deinit();

void Gui_InputData(InputData data);

#define BUTTON_HEIGHT 20
#define BUTTON_TEXT_PADDING ((BUTTON_HEIGHT - CHAR_HEIGHT) / 2)
bool Gui_Button(int x, int y, int w, const char* text);

bool Gui_IsCursorInside(int x, int y, int w, int h);
bool Gui_WasCursorInside(int x, int y, int w, int h);
void Gui_GetCursorMovement(int* x, int* y);
bool Gui_EnteredCursorInside(int x, int y, int w, int h);    