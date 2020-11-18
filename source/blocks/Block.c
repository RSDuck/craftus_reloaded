#include <blocks/Block.h>

#include <rendering/TextureMap.h>
#include <rendering/VertexFmt.h>

static Texture_Map textureMap;

// PATH PREFIX
#define PPRX "romfs:/textures/blocks/"

#define TEXTURE_FILES                                                                                                              \
	A(stone, "stone.png")                                                                                                      \
	, A(dirt, "dirt.png"), A(cobblestone, "cobblestone.png"), A(grass_side, "grass_side.png"), A(grass_top, "grass_top.png"),  \
	    A(stonebrick, "stonebrick.png"), A(sand, "sand.png"), A(oaklog_side, "log_oak.png"), A(oaklog_top, "log_oak_top.png"), \
	    A(leaves_oak, "leaves_oak.png"), A(glass, "glass.png"), A(brick, "brick.png"), A(oakplanks, "planks_oak.png"),         \
	    A(wool, "wool.png"), A(bedrock, "bedrock.png")

#define A(i, n) PPRX n
const char* texture_files[] = {TEXTURE_FILES};
#undef A

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
	Texture_MapInit(&textureMap, texture_files, sizeof(texture_files) / sizeof(texture_files[0]));
#define A(i, n) icon.i = Texture_MapGetIcon(&textureMap, PPRX n)
	TEXTURE_FILES;
#undef A
}
void Block_Deinit() { C3D_TexDelete(&textureMap.texture); }

void* Block_GetTextureMap() { return &textureMap.texture; }

void Block_GetTexture(Block block, Direction direction, uint8_t metadata, int16_t* out_uv) {
	Texture_MapIcon i = {0, 0, 0};
	switch (block) {
		case Block_Air:
			return;
		case Block_Dirt:
			i = icon.dirt;
			break;
		case Block_Stone:
			i = icon.stone;
			break;
		case Block_Grass:
			switch (direction) {
				case Direction_Top:
					i = icon.grass_top;
					break;
				case Direction_Bottom:
					i = icon.dirt;
					break;
				default:
					i = icon.grass_side;
					break;
			}
			break;
		case Block_Cobblestone:
			i = icon.cobblestone;
			break;
		case Block_Log:
			switch (direction) {
				case Direction_Bottom:
				case Direction_Top:
					i = icon.oaklog_top;
					break;
				default:
					i = icon.oaklog_side;
					break;
			}
			break;
		case Block_Sand:
			i = icon.sand;
			break;
		case Block_Leaves:
			i = icon.leaves_oak;
			break;
		case Block_Glass:
			i = icon.glass;
			break;
		case Block_Stonebrick:
			i = icon.stonebrick;
			break;
		case Block_Brick:
			i = icon.brick;
			break;
		case Block_Planks:
			i = icon.oakplanks;
			break;
		case Block_Wool:
			i = icon.wool;
			break;
		case Block_Bedrock:
			i = icon.bedrock;
			break;
		default: break;
	}
	out_uv[0] = i.u;
	out_uv[1] = i.v;
}

#define extractR(c) ((c >> 16) & 0xff)
#define extractG(c) (((c) >> 8) & 0xff)
#define extractB(c) ((c)&0xff)
/*#define toRGB16(c) \
	{ extractR(c), extractG(c), extractB(c) }*/
void Block_GetColor(Block block, uint8_t metadata, Direction direction, uint8_t out_rgb[]) {
	if ((block == Block_Grass && direction == Direction_Top) || block == Block_Leaves) {
		out_rgb[0] = 140;
		out_rgb[1] = 214;
		out_rgb[2] = 123;
		return;
	}
	// white, orange, magenta, light blue, yellow, lime, pink, gray, silver, cyan, purple, blue, green, red, black
	const uint32_t dies[] = {(16777215), (14188339), (11685080), (6724056), (15066419), (8375321), (15892389), (5000268),
				 (10066329), (5013401),  (8339378),  (3361970), (6704179),  (6717235), (10040115), (1644825)};
	if (block == Block_Wool) {
		out_rgb[0] = extractR(dies[metadata]);
		out_rgb[1] = extractG(dies[metadata]);
		out_rgb[2] = extractB(dies[metadata]);
	} else {
		out_rgb[0] = 255;
		out_rgb[1] = 255;
		out_rgb[2] = 255;
	}
}

bool Block_Opaque(Block block, uint8_t metadata) { return block != Block_Air && block != Block_Leaves && block != Block_Glass; }

const char* BlockNames[Blocks_Count] = {"Air",    "Stone", "Dirt",	 "Grass",  "Cobblestone", "Sand", "Log",
					"Leaves", "Glass", "Stone Bricks", "Bricks", "Planks",      "Wool", "Bedrock"};
