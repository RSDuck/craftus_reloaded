#include <gui/SpriteBatch.h>

#include <gui/DebugUI.h>
#include <gui/FontLoader.h>
#include <rendering/TextureMap.h>
#include <rendering/VertexFmt.h>

#include <stdarg.h>

#include <vec/vec.h>

typedef struct {
	int depth;
	C3D_Tex* texture;
	int16_t x0, y0, x1, y1;  // top left, right
	int16_t x2, y2, x3, y3;  // bottom left, right
	int16_t u0, v0, u1, v1;
	int16_t color;
} Sprite;

static vec_t(Sprite) cmdList;
static C3D_Tex* currentTexture = NULL;
static GuiVertex* vertexList[2];
static int projUniform;

static Font* font;
static C3D_Tex whiteTex;
static C3D_Tex widgetsTex;
static C3D_Tex iconsTex;
static C3D_Tex menuBackgroundTex;

static int screenWidth = 0, screenHeight = 0;
static int guiScale = 2;

void SpriteBatch_Init(int projUniform_) {
	vec_init(&cmdList);

	vertexList[0] = linearAlloc(sizeof(GuiVertex) * 256);
	vertexList[1] = linearAlloc(sizeof(GuiVertex) *2 * (4096 + 1024));

	projUniform = projUniform_;

	font = (Font*)malloc(sizeof(Font));
	FontLoader_Init(font, "romfs:/textures/font/ascii.png");
	Texture_Load(&widgetsTex, "romfs:/textures/gui/widgets.png");

	uint8_t data[16 * 16];
	memset(data, 0xff, 16 * 16 * sizeof(uint8_t));
	C3D_TexInit(&whiteTex, 16, 16, GPU_L8);
	C3D_TexLoadImage(&whiteTex, data, GPU_TEXFACE_2D, 0);

	Texture_Load(&menuBackgroundTex, "romfs:/textures/gui/options_background.png");
}
void SpriteBatch_Deinit() {
	vec_deinit(&cmdList);
	linearFree(vertexList[0]);
	linearFree(vertexList[1]);

	C3D_TexDelete(&font->texture);
	free(font);

	C3D_TexDelete(&whiteTex);
	C3D_TexDelete(&widgetsTex);
	C3D_TexDelete(&menuBackgroundTex);
}

void SpriteBatch_BindTexture(C3D_Tex* texture) { currentTexture = texture; }
void SpriteBatch_BindGuiTexture(GuiTexture texture) {
	switch (texture) {
		case GuiTexture_Blank:
			currentTexture = &whiteTex;
			break;
		case GuiTexture_Font:
			currentTexture = &font->texture;
			break;
		case GuiTexture_Widgets:
			currentTexture = &widgetsTex;
			break;
		case GuiTexture_Icons:
			currentTexture = &iconsTex;
			break;
		case GuiTexture_MenuBackground:
			currentTexture = &menuBackgroundTex;
			break;
		default:
			break;
	}
}

void SpriteBatch_PushSingleColorQuad(int x, int y, int z, int w, int h, int16_t color) {
	SpriteBatch_BindTexture(&whiteTex);
	SpriteBatch_PushQuadColor(x, y, z, w, h, 0, 0, 4, 4, color);
}
void SpriteBatch_PushQuad(int x, int y, int z, int w, int h, int rx, int ry, int rw, int rh) {
	SpriteBatch_PushQuadColor(x, y, z, w, h, rx, ry, rw, rh, INT16_MAX);
}
void SpriteBatch_PushQuadColor(int x, int y, int z, int w, int h, int rx, int ry, int rw, int rh, int16_t color) {
	vec_push(&cmdList, ((Sprite){z, currentTexture, x * guiScale, y * guiScale, (x + w) * guiScale, y * guiScale, x * guiScale,
				     (y + h) * guiScale, (x + w) * guiScale, (y + h) * guiScale, rx, ry, rx + rw, ry + rh, color}));
}
void SpriteBatch_PushIcon(Block block, uint8_t metadata, int x, int y, int z) {
	int16_t uvs[2];
	uint8_t color[3];
	C3D_Tex* texture = Block_GetTextureMap();
	Block_GetTexture(block, Direction_Top, metadata, uvs);
	Block_GetColor(block, metadata, Direction_Top, color);
	int16_t u = uvs[0] / 256;
	int16_t v = uvs[1] / 256;
	vec_push(&cmdList,
		 ((Sprite){z, texture, (2 + x) * guiScale, (7 + y) * guiScale, (16 + x) * guiScale, (1 + y) * guiScale, (16 + x) * guiScale,
			   (14 + y) * guiScale, (30 + x) * guiScale, (7 + y) * guiScale, u, v + TEXTURE_TILESIZE, u + TEXTURE_TILESIZE, v,
			   SHADER_RGB(color[0] >> 3, color[1] >> 3, color[2] >> 3)}));
	Block_GetTexture(block, Direction_North, metadata, uvs);
	Block_GetColor(block, metadata, Direction_North, color);
	u = uvs[0] / 256;
	v = uvs[1] / 256;
	vec_push(&cmdList,
		 ((Sprite){z, texture, (2 + x) * guiScale, (7 + y) * guiScale, (16 + x) * guiScale, (14 + y) * guiScale, (2 + x) * guiScale,
			   (25 + y) * guiScale, (16 + x) * guiScale, (31 + y) * guiScale, u, v + TEXTURE_TILESIZE, u + TEXTURE_TILESIZE, v,
			   SHADER_RGB_DARKEN(SHADER_RGB(color[0] >> 3, color[1] >> 3, color[2] >> 3), 11)}));

	Block_GetTexture(block, Direction_East, metadata, uvs);
	Block_GetColor(block, metadata, Direction_East, color);
	u = uvs[0] / 256;
	v = uvs[1] / 256;
	vec_push(&cmdList,
		 ((Sprite){z, texture, (16 + x) * guiScale, (14 + y) * guiScale, (30 + x) * guiScale, (7 + y) * guiScale,
			   (16 + x) * guiScale, (31 + y) * guiScale, (30 + x) * guiScale, (25 + y) * guiScale, u, v + TEXTURE_TILESIZE,
			   u + TEXTURE_TILESIZE, v, SHADER_RGB_DARKEN(SHADER_RGB(color[0] >> 3, color[1] >> 3, color[2] >> 3), 13)}));
}

int SpriteBatch_PushText(int x, int y, int z, int16_t color, bool shadow, int wrap, int* ySize, const char* fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	int length = SpriteBatch_PushTextVargs(x, y, z, color, shadow, wrap, ySize, fmt, arg);
	va_end(arg);
	return length;
}

int SpriteBatch_PushTextVargs(int x, int y, int z, int16_t color, bool shadow, int wrap, int* ySize, const char* fmt, va_list arg) {
	SpriteBatch_BindTexture(&font->texture);
#define CHAR_WIDTH 8
#define TAB_SIZE 4

	char buffer[256];
	vsprintf(buffer, fmt, arg);

	int offsetX = 0;
	int offsetY = 0;

	int maxWidth = 0;

	char* text = buffer;

	while (*text != '\0') {
		bool implicitBreak = offsetX + font->fontWidth[(int)*text] >= wrap;
		if (*text == '\n' || implicitBreak) {
			offsetY += CHAR_HEIGHT;
			maxWidth = MAX(maxWidth, offsetX);
			offsetX = 0;
			if (implicitBreak) --text;
		} else if (*text == '\t') {
			offsetX = ((offsetX / CHAR_WIDTH) / TAB_SIZE + 1) * TAB_SIZE * CHAR_WIDTH;
		} else {
			if (*text != ' ') {
				int texX = *text % 16 * 8, texY = *text / 16 * 8;
				SpriteBatch_PushQuadColor(x + offsetX, y + offsetY, z, 8, 8, texX, texY, 8, 8, color);
				if (shadow)
					SpriteBatch_PushQuadColor(x + offsetX + 1, y + offsetY + 1, z - 1, 8, 8, texX, texY, 8, 8,
								  SHADER_RGB(10, 10, 10));
			}
			offsetX += font->fontWidth[(int)*text];
		}
		++text;
	}

	maxWidth = MAX(maxWidth, offsetX);

	if (ySize != NULL) *ySize = offsetY + CHAR_HEIGHT;

	return maxWidth;
}

int SpriteBatch_CalcTextWidth(const char* text, ...) {
	va_list args;
	va_start(args, text);
	int length = SpriteBatch_CalcTextWidthVargs(text, args);
	va_end(args);

	return length;
}

int SpriteBatch_CalcTextWidthVargs(const char* text, va_list args) {
	char fmtedText[256];
	vsprintf(fmtedText, text, args);

	char* it = fmtedText;

	int length = 0;
	int maxLength = 0;
	while (*it != '\0') {
		if (*it == '\n') {
			maxLength = MAX(maxLength, length);
			length = 0;
		} else
			length += font->fontWidth[(int)*(it++)];
	}

	maxLength = MAX(maxLength, length);

	return maxLength;
}

static int compareDrawCommands(const void* a, const void* b) {
	Sprite* ga = ((Sprite*)a);
	Sprite* gb = ((Sprite*)b);

	return ga->depth == gb->depth ? gb->texture - ga->texture : gb->depth - ga->depth;
}

int SpriteBatch_GetWidth() { return screenWidth / guiScale; }
int SpriteBatch_GetHeight() { return screenHeight / guiScale; }

void SpriteBatch_SetScale(int scale) { guiScale = scale; }
int SpriteBatch_GetScale() { return guiScale; }

void SpriteBatch_StartFrame(int width, int height) {
	screenWidth = width;
	screenHeight = height;
}

void SpriteBatch_Render(gfxScreen_t screen) {
	vec_sort(&cmdList, &compareDrawCommands);

	C3D_Mtx projMtx;
	Mtx_OrthoTilt(&projMtx, 0.f, screenWidth, screenHeight, 0.f, 1.f, -1.f, false);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &projMtx);

	C3D_DepthTest(false, GPU_GREATER, GPU_WRITE_ALL);

	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
	C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

	GuiVertex* usedVertexList = vertexList[screen];

	int verticesTotal = 0;

	size_t vtx = 0;
	while (cmdList.length > 0) {
		size_t vtxStart = vtx;

		C3D_Tex* texture = vec_last(&cmdList).texture;
		float divW = 1.f / texture->width * INT16_MAX, divH = 1.f / texture->height * INT16_MAX;

		while (cmdList.length > 0 && vec_last(&cmdList).texture == texture) {
			Sprite cmd = vec_pop(&cmdList);
			int16_t color = cmd.color;

			int16_t u0 = (int16_t)((float)cmd.u0 * divW), v0 = (int16_t)((float)cmd.v0 * divH);
			int16_t u1 = (int16_t)((float)cmd.u1 * divW), v1 = (int16_t)((float)cmd.v1 * divH);

			usedVertexList[vtx++] = (GuiVertex){{cmd.x3, cmd.y3, 0}, {u1, v1, color}};
			usedVertexList[vtx++] = (GuiVertex){{cmd.x1, cmd.y1, 0}, {u1, v0, color}};
			usedVertexList[vtx++] = (GuiVertex){{cmd.x0, cmd.y0, 0}, {u0, v0, color}};

			usedVertexList[vtx++] = (GuiVertex){{cmd.x0, cmd.y0, 0}, {u0, v0, color}};
			usedVertexList[vtx++] = (GuiVertex){{cmd.x2, cmd.y2, 0}, {u0, v1, color}};
			usedVertexList[vtx++] = (GuiVertex){{cmd.x3, cmd.y3, 0}, {u1, v1, color}};
		}

		C3D_TexBind(0, texture);

		C3D_BufInfo* bufInfo = C3D_GetBufInfo();
		BufInfo_Init(bufInfo);
		BufInfo_Add(bufInfo, usedVertexList + vtxStart, sizeof(GuiVertex), 2, 0x10);

		C3D_DrawArrays(GPU_TRIANGLES, 0, vtx - vtxStart);

		verticesTotal += vtx - vtxStart;
	}
	C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);

	currentTexture = NULL;
	guiScale = 2;
}