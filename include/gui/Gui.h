#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <misc/InputData.h>

void Gui_Init();
void Gui_Deinit();

void Gui_InputData(InputData data);

void Gui_Frame();

int Gui_RelativeWidth(float x);
int Gui_RelativeHeight(float y);

void Gui_Offset(int x, int y);
void Gui_BeginRowCenter(int width, int count);
void Gui_VerticalSpace(int y);

void Gui_Space(float space);

void Gui_BeginRow(int width, int count);
void Gui_EndRow();

void Gui_Label(float size, bool shadow, int16_t color, bool center, const char* text, ...); // size[0-1] Anteil
#define BUTTON_HEIGHT 20
#define BUTTON_TEXT_PADDING ((BUTTON_HEIGHT - CHAR_HEIGHT) / 2)
bool Gui_Button(float size, const char* label, ...); // size[0-1] Anteil an derzeitiger Reihe

bool Gui_IsCursorInside(int x, int y, int w, int h);
bool Gui_WasCursorInside(int x, int y, int w, int h);
void Gui_GetCursorMovement(int* x, int* y);
bool Gui_EnteredCursorInside(int x, int y, int w, int h);