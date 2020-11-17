#include <rendering/Cursor.h>

#include <rendering/VertexFmt.h>

#include <misc/NumberUtils.h>

static WorldVertex* cursorVBO;

extern const WorldVertex cube_sides_lut[6 * 6];

void Cursor_Init() {
	cursorVBO = linearAlloc(sizeof(cube_sides_lut));
	memcpy(cursorVBO, cube_sides_lut, sizeof(cube_sides_lut));

	for (int i = 0; i < 6 * 6; i++) {
		cursorVBO[i].rgb[0] = 255;
		cursorVBO[i].rgb[1] = 255;
		cursorVBO[i].rgb[2] = 255;
	}
}

void Cursor_Deinit() { linearFree(cursorVBO); }

void Cursor_Draw(int projUniform, C3D_Mtx* projectionview, World* world, int x, int y, int z, Direction highlight) {
	C3D_Mtx model;
	Mtx_Identity(&model);
	Mtx_Translate(&model, x, y, z, true);

	C3D_Mtx mvp;
	Mtx_Multiply(&mvp, projectionview, &model);

	size_t vertices = 0;
	WorldVertex* vtx = cursorVBO;
	for (int i = 0; i < 6; i++) {
		const int* offset = DirectionToOffset[i];
		if (World_GetBlock(world, x + offset[0], y + offset[1], z + offset[2]) == Block_Air) {
			memcpy(vtx, &cube_sides_lut[i * 6], sizeof(WorldVertex) * 6);
			uint8_t color = i == highlight ? 65 : 32;
			for (int j = 0; j < 6; j++) {
				vtx[j].rgb[0] = color;
				vtx[j].rgb[1] = color;
				vtx[j].rgb[2] = color;
			}
			vtx += 6;
			vertices += 6;
		}
	}

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &mvp);

	C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ZERO, GPU_ONE_MINUS_SRC_ALPHA);

	C3D_DepthMap(true, -1.f, 0.001f);

	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, cursorVBO, sizeof(WorldVertex), 4, 0x3210);

	C3D_DrawArrays(GPU_TRIANGLES, 0, vertices);

	C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);

	C3D_DepthMap(true, -1.f, 0.0f);
}