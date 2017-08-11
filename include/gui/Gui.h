#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include <misc/InputData.h>
#include <world/World.h>

#include <citro3d.h>

void Gui_Init(int projUniform_);
void Gui_Deinit();

void Gui_InputData(InputData data);

typedef enum { GuiTexture_Blank, GuiTexture_Font, GuiTexture_Icons, GuiTexture_Widgets } GuiTexture;

void Gui_BindGuiTexture(GuiTexture texture);
void Gui_BindTexture(C3D_Tex* texture);

void Gui_PushSingleColorQuad(int x, int y, int z, int w, int h, int16_t color);
void Gui_PushQuadColor(int x, int y, int z, int w, int h, int rx, int ry, int rw, int rh, int16_t color);
void Gui_PushQuad(int x, int y, int z, int w, int h, int rx, int ry, int rw, int rh);
void Gui_PushIcon(Block block, int x, int y, int z, int size);

#define CHAR_HEIGHT 8
int Gui_PushTextVargs(int x, int y, int z, int16_t color, bool shadow, int wrap, int* ySize, const char* fmt, va_list arg);
int Gui_PushText(int x, int y, int z, int16_t color, bool shadow, int wrap, int* ySize, const char* fmt, ...);

int Gui_CalcTextWidthVargs(const char* text, va_list args);
int Gui_CalcTextWidth(const char* text, ...);

bool Gui_IsCursorInside(int x, int y, int w, int h);
bool Gui_WasCursorInside(int x, int y, int w, int h);
void Gui_GetCursorMovement(int* x, int* y);

bool Gui_RectIsVisible(int x, int y, int w, int h);

#define BUTTON_HEIGHT 20
bool Gui_Button(int x, int y, int w, const char* text);

void Gui_SetScale(int scale);

void Gui_Render(gfxScreen_t screen);