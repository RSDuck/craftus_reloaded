#include <blocks/Block.h>

#include <rendering/TextureMap.h>
#include <rendering/VertexFmt.h>

static Texture_Map textureMap;

// PATH PREFIX
#define PPRX "romfs:/textures/blocks/"

const char* files[] = {PPRX "stone.png",      PPRX "dirt.png",  PPRX "cobblestone.png", PPRX "grass_side.png", PPRX "grass_top.png",
		       PPRX "stonebrick.png", PPRX "sand.png",  PPRX "log_oak_top.png", PPRX "log_oak.png",    PPRX "leaves_oak.png",
		       PPRX "glass.png",      PPRX "brick.png", PPRX "planks_oak.png",  PPRX "wool.png",       PPRX "bedrock.png"};

static struct {
	Texture_MapIcon stone;
	Texture_MapIcon dirt;
	Texture_MapIcon cobblestone;
	Texture_MapIcon grass_side;
	Texture_MapIcon grass_top;
	Texture_MapIcon stonebrick;
	Texture_MapIcon sand;
	Texture_MapIcon oaklog_side;
	Texture_MapIcon oaklog_top;
	Texture_MapIcon leaves_oak;
	Texture_MapIcon glass;
	Texture_MapIcon brick;
	Texture_MapIcon oakplanks;
	Texture_MapIcon wool;
	Texture_MapIcon bedrock;
} icon;

void Block_Init() {
	Texture_MapInit(&textureMap, files, 15);
#define A(i, n) icon.i = Texture_MapGetIcon(&textureMap, PPRX n)
	A(stone, "stone.png");
	A(dirt, "dirt.png");
	A(cobblestone, "cobblestone.png");
	A(grass_side, "grass_side.png");
	A(grass_top, "grass_top.png");
	A(stonebrick, "stonebrick.png");
	A(sand, "sand.png");
	A(oaklog_side, "log_oak.png");
	A(oaklog_top, "log_oak_top.png");
	A(leaves_oak, "leaves_oak.png");
	A(glass, "glass.png");
	A(brick, "brick.png");
	A(oakplanks, "planks_oak.png");
	A(wool, "wool.png");
	A(bedrock, "bedrock.png");
#undef A
}
void Block_Deinit() { C3D_TexDelete(&textureMap.texture); }

void* Block_GetTextureMap() { return &textureMap.texture; }

void Block_GetTexture(Block block, Direction direction, int16_t* out_uv) {
	switch (block) {
		case Block_Air:
			return;
		case Block_Dirt:
			out_uv[0] = icon.dirt.u;
			out_uv[1] = icon.dirt.v;
			return;
		case Block_Stone:
			out_uv[0] = icon.stone.u;
			out_uv[1] = icon.stone.v;
			return;
		case Block_Grass:
			switch (direction) {
				case Direction_Top:
					out_uv[0] = icon.grass_top.u;
					out_uv[1] = icon.grass_top.v;
					return;
				case Direction_Bottom:
					out_uv[0] = icon.dirt.u;
					out_uv[1] = icon.dirt.v;
					return;
				default:
					out_uv[0] = icon.grass_side.u;
					out_uv[1] = icon.grass_side.v;
					return;
			}
		case Block_Cobblestone:
			out_uv[0] = icon.cobblestone.u;
			out_uv[1] = icon.cobblestone.v;
			return;
		case Block_Log:
			switch (direction) {
				case Direction_Bottom:
				case Direction_Top:
					out_uv[0] = icon.oaklog_top.u;
					out_uv[1] = icon.oaklog_top.v;
					return;
				default:
					out_uv[0] = icon.oaklog_side.u;
					out_uv[1] = icon.oaklog_side.v;
					return;
			}
		case Block_Sand:
			out_uv[0] = icon.sand.u;
			out_uv[1] = icon.sand.v;
			return;
		case Block_Leaves:
			out_uv[0] = icon.leaves_oak.u;
			out_uv[1] = icon.leaves_oak.v;
			return;
		case Block_Glass:
			out_uv[0] = icon.glass.u;
			out_uv[1] = icon.glass.v;
			return;
		case Block_Stonebrick:
			out_uv[0] = icon.stonebrick.u;
			out_uv[1] = icon.stonebrick.v;
			return;
		case Block_Brick:
			out_uv[0] = icon.brick.u;
			out_uv[1] = icon.brick.v;
			return;
		case Block_Planks:
			out_uv[0] = icon.oakplanks.u;
			out_uv[1] = icon.oakplanks.v;
			return;
		case Block_Wool:
			out_uv[0] = icon.wool.u;
			out_uv[1] = icon.wool.v;
			return;
		case Block_Bedrock:
			out_uv[0] = icon.bedrock.u;
			out_uv[1] = icon.bedrock.v;
			return;
	}
}

uint16_t Block_GetColor(Block block, Direction direction) {
	if ((block == Block_Grass && direction == Direction_Top) || block == Block_Leaves) {
		return SHADER_RGB(17, 26, 15);
	}
	return SHADER_RGB(31, 31, 31);
}

bool Block_Opaque(Block block) { return block != Block_Air && block != Block_Leaves && block != Block_Glass; }

const char* BlockNames[Blocks_Count] = {"Air",    "Stone", "Dirt",	 "Grass",  "Cobblestone", "Sand", "Log",
					"Leaves", "Glass", "Stone Bricks", "Bricks", "Planks",      "Wool", "Bedrock"};
