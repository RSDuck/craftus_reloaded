#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>


#include <misc/InputData.h>
#include <world/World.h>

#include <citro3d.h>

void SpriteBatch_Init(int projUniform_);
void SpriteBatch_Deinit();

typedef enum { GuiTexture_Blank, GuiTexture_Font, GuiTexture_Icons, GuiTexture_Widgets, GuiTexture_MenuBackground } GuiTexture;

void SpriteBatch_BindGuiTexture(GuiTexture texture);
void SpriteBatch_BindTexture(C3D_Tex* texture);

void SpriteBatch_PushSingleColorQuad(int x, int y, int z, int w, int h, int16_t color);
void SpriteBatch_PushQuadColor(int x, int y, int z, int w, int h, int rx, int ry, int rw, int rh, int16_t color);
void SpriteBatch_PushQuad(int x, int y, int z, int w, int h, int rx, int ry, int rw, int rh);
void SpriteBatch_PushIcon(Block block, uint8_t metadata, int x, int y, int z);

#define CHAR_HEIGHT 8
int SpriteBatch_PushTextVargs(int x, int y, int z, int16_t color, bool shadow, int wrap, int* ySize, const char* fmt, va_list arg);
int SpriteBatch_PushText(int x, int y, int z, int16_t color, bool shadow, int wrap, int* ySize, const char* fmt, ...);

int SpriteBatch_CalcTextWidthVargs(const char* text, va_list args);
int SpriteBatch_CalcTextWidth(const char* text, ...);

bool SpriteBatch_RectIsVisible(int x, int y, int w, int h);

void SpriteBatch_SetScale(int scale);
int SpriteBatch_GetScale();

int SpriteBatch_GetWidth();
int SpriteBatch_GetHeight();

void SpriteBatch_StartFrame(int width, int height);
void SpriteBatch_Render(gfxScreen_t screen);