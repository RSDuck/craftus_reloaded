#include <rendering/Hand.h>

#include <misc/NumberUtils.h>
#include <rendering/VertexFmt.h>

#include <rendering/TextureMap.h>

#include <string.h>

static WorldVertex* handVBO;
static C3D_Tex steveTexture;

extern const WorldVertex cube_sides_lut[6 * 6];

void Hand_Init() {
	handVBO = linearAlloc(sizeof(cube_sides_lut));
	Texture_Load(&steveTexture, "romfs:/textures/entity/steve.png");
}
void Hand_Deinit() {
	linearFree(handVBO);
	C3D_TexDelete(&steveTexture);
}

void Hand_Draw(int projUniform, C3D_Mtx* projection, ItemStack stack, Player* player) {
	C3D_Mtx pm;
	C3D_Mtx model;
	Mtx_Identity(&model);
	Mtx_Translate(&model, 0.5f + sinf(player->bobbing) * 0.03f + (stack.amount == 0) * 0.2f,
		      -0.68f + ABS(sinf(player->bobbing)) * 0.01f, -1.2f - (stack.amount == 0) * 0.4f, true);
	Mtx_RotateX(&model, M_PI / 18.f + (stack.amount == 0) * M_PI / 5.f, true);
	Mtx_RotateY(&model, M_PI / 18.f, true);
	if (player->breakPlaceTimeout > -0.1f) {
		float dist = sinf((player->breakPlaceTimeout + 0.1f) / (PLAYER_PLACE_REPLACE_TIMEOUT + 0.1f) * M_PI);
		float h = cosf((player->breakPlaceTimeout + 0.1f) / (PLAYER_PLACE_REPLACE_TIMEOUT + 0.1f) * M_PI);
		Mtx_RotateX(&model, -dist, true);
		Mtx_Translate(&model, 0.f, -h * 0.3f, -dist * 0.25f, true);
	}
	if (stack.amount == 0) Mtx_RotateZ(&model, DEG_TO_RAD * 40.f, true);
	Mtx_Scale(&model, 0.28f, 0.28f, stack.amount == 0 ? 0.8f : 0.28f);

	Mtx_Multiply(&pm, projection, &model);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &pm);

	memcpy(handVBO, cube_sides_lut, sizeof(cube_sides_lut));
	for (int i = 0; i < 6; i++) {
		if (stack.amount > 0) {
			int16_t iconUV[2];
			uint8_t color[3];
			Block_GetTexture(stack.block, i, stack.meta, iconUV);
			Block_GetColor(stack.block, stack.meta, i, color);

#define oneDivIconsPerRow (32768 / 8)
#define halfTexel (6)

			for (int j = 0; j < 6; j++) {
				int idx = i * 6 + j;

				handVBO[idx].uv[0] = (handVBO[idx].uv[0] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[0];
				handVBO[idx].uv[1] = (handVBO[idx].uv[1] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[1];

				handVBO[idx].rgb[0] = color[0];
				handVBO[idx].rgb[1] = color[1];
				handVBO[idx].rgb[2] = color[2];
			}
		} else {
			C3D_TexBind(0, &steveTexture);

			if (i == Direction_East ||
			    i == Direction_West) {  // eines der dümmsten Dinge, die ich jemals in meinem Leben getan habe
				const int16_t uvRotationTable[2][2][2][2] = {
				    {{{0, 1}, {0, 0}}, {{1, 1}, {1, 0}}}, {{{1, 0}, {1, 1}}, {{0, 0}, {0, 1}}},
				};
				for (int j = 0; j < 6; j++) {
					int idx = i * 6 + j;
					int u = handVBO[idx].uv[0];
					int v = handVBO[idx].uv[1];
					handVBO[idx].uv[0] = uvRotationTable[(i == Direction_West) ? 1 : 0][v][u][0];
					handVBO[idx].uv[1] = uvRotationTable[(i == Direction_East) ? 1 : 0][v][u][1];
				}
			}
			for (int j = 0; j < 6; j++) {
				int idx = i * 6 + j;
#define toTexCoord(x, tw) (int16_t)(((float)(x) / (float)(tw)) * (float)(1 << 15))
				const int16_t uvLookUp[6][4] = {
				    {toTexCoord(48, 64), toTexCoord(52, 64), toTexCoord(20, 64), toTexCoord(32, 64)},  // west = inside
				    {toTexCoord(40, 64), toTexCoord(44, 64), toTexCoord(20, 64), toTexCoord(32, 64)},  // east = outside
				    {toTexCoord(52, 64), toTexCoord(56, 64), toTexCoord(20, 64), toTexCoord(32, 64)},  // bottom = back
				    {toTexCoord(44, 64), toTexCoord(48, 64), toTexCoord(20, 64), toTexCoord(32, 64)},  // top = front
				    {toTexCoord(48, 64), toTexCoord(52, 64), toTexCoord(16, 64), toTexCoord(20, 64)},  // south = bottom
				    {toTexCoord(44, 64), toTexCoord(48, 64), toTexCoord(16, 64), toTexCoord(20, 64)},  // north = top
				};

				handVBO[idx].uv[0] = uvLookUp[i][handVBO[idx].uv[0]];
				handVBO[idx].uv[1] = uvLookUp[i][handVBO[idx].uv[1] + 2];
			}
		}
	}

	C3D_AlphaTest(true, GPU_GEQUAL, 255);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, handVBO, sizeof(WorldVertex), 4, 0x3210);

	C3D_DrawArrays(GPU_TRIANGLES, 0, 6 * 6);

	C3D_AlphaTest(false, GPU_GREATER, 0);
}