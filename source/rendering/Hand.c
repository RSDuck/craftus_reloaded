#include <rendering/Hand.h>

#include <misc/NumberUtils.h>
#include <rendering/VertexFmt.h>

#include <string.h>

static Vertex* handVBO;

extern const Vertex cube_sides_lut[6 * 6];

void Hand_Init() { handVBO = linearAlloc(sizeof(cube_sides_lut)); }
void Hand_Deinit() { linearFree(handVBO); }

void Hand_Draw(int projUniform, C3D_Mtx* projection, Block block, Player* player) {
	C3D_Mtx pm;
	C3D_Mtx model;
	Mtx_Identity(&model);
	Mtx_Translate(&model, 0.5f + sinf(player->bobbing) * 0.03f, -0.68f + ABS(sinf(player->bobbing)) * 0.01f, -1.2f, true);
	Mtx_RotateX(&model, M_PI / 18.f, true);
	Mtx_RotateY(&model, M_PI / 18.f, true);
	if (player->breakPlaceTimeout > -0.1f) {
		float dist = sinf((player->breakPlaceTimeout + 0.1f) / (PLAYER_PLACE_REPLACE_TIMEOUT + 0.1f) * M_PI);
		float h = cosf((player->breakPlaceTimeout + 0.1f) / (PLAYER_PLACE_REPLACE_TIMEOUT + 0.1f) * M_PI);
		Mtx_Translate(&model, 0.f, -h * 0.3f, -dist * 0.5f, true);
	}
	Mtx_Scale(&model, 0.35f, 0.35f, 0.35f);

	Mtx_Multiply(&pm, projection, &model);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &pm);

	memcpy(handVBO, cube_sides_lut, sizeof(cube_sides_lut));
	for (int i = 0; i < 6; i++) {
		int16_t iconUV[2];
		Block_GetTexture(block, i, iconUV);
		uint16_t color = Block_GetColor(block, i);

#define oneDivIconsPerRow (32768 / 8)
#define halfTexel (6)

		for (int j = 0; j < 6; j++) {
			handVBO[i * 6 + j].uvc[0] = (handVBO[i * 6 + j].uvc[0] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[0];
			handVBO[i * 6 + j].uvc[1] = (handVBO[i * 6 + j].uvc[1] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[1];

			handVBO[i * 6 + j].uvc[2] = color;
		}
	}

	C3D_AlphaTest(true, GPU_GEQUAL, 255);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, handVBO, sizeof(Vertex), 2, 0x10);

	C3D_DrawArrays(GPU_TRIANGLES, 0, 6 * 6);

	C3D_AlphaTest(false, GPU_GREATER, 0);
}