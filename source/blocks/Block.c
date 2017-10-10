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
	Texture_MapIcon i;
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
	}
	out_uv[0] = i.u;
	out_uv[1] = i.v;
}

#define extractR(c) ((c >> 16) & 0xff)
#define extractG(c) (((c) >> 8) & 0xff)
#define extractB(c) ((c)&0xff)
#define toRGB16(c) SHADER_RGB(extractR(c), extractG(c), extractB(c))
uint16_t Block_GetColor(Block block, uint8_t metadata, Direction direction) {
	if ((block == Block_Grass && direction == Direction_Top) || block == Block_Leaves) {
		return SHADER_RGB(17, 26, 15);
	}
	// white, orange, magenta, light blue, yellow, lime, pink, gray, silver, cyan, purple, blue, green, red, black
	const uint16_t dies[] = {toRGB16(16777215), toRGB16(14188339), toRGB16(11685080), toRGB16(6724056),
				 toRGB16(15066419), toRGB16(8375321),  toRGB16(15892389), toRGB16(5000268),
				 toRGB16(10066329), toRGB16(5013401),  toRGB16(8339378),  toRGB16(3361970),
				 toRGB16(6704179),  toRGB16(6717235),  toRGB16(10040115), toRGB16(1644825)};
	if (block == Block_Wool) return dies[metadata];
	return SHADER_RGB(31, 31, 31);
}

bool Block_Opaque(Block block, uint8_t metadata) { return block != Block_Air && block != Block_Leaves && block != Block_Glass; }

const char* BlockNames[Blocks_Count] = {"Air",    "Stone", "Dirt",	 "Grass",  "Cobblestone", "Sand", "Log",
					"Leaves", "Glass", "Stone Bricks", "Bricks", "Planks",      "Wool", "Bedrock"};
