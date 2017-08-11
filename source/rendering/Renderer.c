#include <rendering/Renderer.h>

#include <blocks/Block.h>
#include <gui/Gui.h>
#include <gui/DebugUI.h>
#include <rendering/Camera.h>
#include <rendering/Cursor.h>
#include <rendering/PolyGen.h>
#include <rendering/TextureMap.h>
#include <rendering/WorldRenderer.h>

#include <citro3d.h>

#include <world_shbin.h>

#define DISPLAY_TRANSFER_FLAGS                                                                                                          \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
	 GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static C3D_RenderTarget* renderTargets[2];
static C3D_RenderTarget* lowerScreen;

static DVLB_s* world_dvlb;
static shaderProgram_s world_shader;
static int world_shader_uLocProjection;

static World* world;
static Player* player;
static WorkQueue* workqueue;

void Renderer_Init(World* world_, Player* player_, WorkQueue* queue) {
	world = world_;
	player = player_;
	workqueue = queue;

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	renderTargets[0] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	renderTargets[1] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetClear(renderTargets[0], C3D_CLEAR_ALL, 0x90d9ffff, 0);
	C3D_RenderTargetSetClear(renderTargets[1], C3D_CLEAR_ALL, 0x90d9ffff, 0);
	C3D_RenderTargetSetOutput(renderTargets[0], GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetSetOutput(renderTargets[1], GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

	lowerScreen = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetClear(lowerScreen, C3D_CLEAR_ALL, 0x000000ff, 0);
	C3D_RenderTargetSetOutput(lowerScreen, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	world_dvlb = DVLB_ParseFile((u32*)world_shbin, world_shbin_size);
	shaderProgramInit(&world_shader);
	shaderProgramSetVsh(&world_shader, &world_dvlb->DVLE[0]);
	C3D_BindProgram(&world_shader);

	world_shader_uLocProjection = shaderInstanceGetUniformLocation(world_shader.vertexShader, "projection");

	C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_SHORT, 3);
	AttrInfo_AddLoader(attrInfo, 1, GPU_SHORT, 3);

	PolyGen_Init(world);

	WorldRenderer_Init(player, world, workqueue, world_shader_uLocProjection);

	Gui_Init(world_shader_uLocProjection);

	C3D_CullFace(GPU_CULL_BACK_CCW);

	Block_Init();
}
void Renderer_Deinit() {
	Block_Deinit();

	PolyGen_Deinit();

	WorldRenderer_Deinit();

	Gui_Deinit();

	shaderProgramFree(&world_shader);
	DVLB_Free(world_dvlb);

	C3D_Fini();
}

static bool text = false;
void Renderer_Render() {
	float iod = osGet3DSliderState() / 4.f;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	PolyGen_Harvest();

	for (int i = 0; i < 2; i++) {
		C3D_FrameDrawOn(renderTargets[i]);

		C3D_TexBind(0, Block_GetTextureMap());
		
		WorldRenderer_Render(!i ? -iod : iod);

		C3D_TexEnv* env = C3D_GetTexEnv(0);
		C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
		C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
		C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

		Gui_BindGuiTexture(GuiTexture_Widgets);
		if (iod == 0.f) Gui_PushQuad(200 / 2 - 16 / 2, 120 / 2 - 16 / 2, 0, 16, 16, 240, 0, 16, 16);

		Gui_Render(GFX_TOP);

		if (iod <= 0.f) break;
	}

	C3D_FrameDrawOn(lowerScreen);

	DebugUI_Draw();

	Gui_Render(GFX_BOTTOM);

	C3D_FrameEnd(0);
}