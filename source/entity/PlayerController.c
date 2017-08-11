#include <entity/PlayerController.h>

#include <misc/NumberUtils.h>

#include <gui/DebugUI.h>

#ifdef _3DS
#include <3ds.h>
#define PLATFORM_BUTTONS 17
const char* platform_key_names[PLATFORM_BUTTONS] = {"Not Set", "A",     "B",     "X",      "Y",      "L",	"R",	"Start",    "Select",
						    "DUp",     "DDown", "DLeft", "DRight", "CircUp", "CircDown", "CircLeft", "CircRight"};
enum { K3DS_Undefined = 0,
       K3DS_A,
       K3DS_B,
       K3DS_X,
       K3DS_Y,
       K3DS_L,
       K3DS_R,
       K3DS_START,
       K3DS_SELECT,
       K3DS_DUP,
       K3DS_DDOWN,
       K3DS_DLEFT,
       K3DS_DRIGHT,
       K3DS_CPAD_UP,
       K3DS_CPAD_DOWN,
       K3DS_CPAD_LEFT,
       K3DS_CPAD_RIGHT };
const PlayerControlScheme platform_default_scheme = {.forward = K3DS_X,
						     .backward = K3DS_B,
						     .strafeLeft = K3DS_L,
						     .strafeRight = K3DS_R,
						     .lookLeft = K3DS_CPAD_LEFT,
						     .lookRight = K3DS_CPAD_RIGHT,
						     .lookUp = K3DS_CPAD_UP,
						     .lookDown = K3DS_CPAD_DOWN,
						     .placeBlock = K3DS_A,
						     .breakBlock = K3DS_Y,
						     .jump = K3DS_DUP,
						     .switchBlockLeft = K3DS_DLEFT,
						     .switchBlockRight = K3DS_DRIGHT};
static void convertPlatformInput(InputData* input, float ctrls[], bool keysdown[], bool keysup[]) {
#define reg_bin_key(i, k)                                                         \
	ctrls[(i)] = (float)((input->keysdown & (k)) || (input->keysheld & (k))); \
	keysdown[(i)] = input->keysdown & (k);                                    \
	keysup[(i)] = input->keysup & (k)

	ctrls[0] = 0.f;
	keysdown[0] = 0;
	keysup[0] = 0;

	reg_bin_key(K3DS_A, KEY_A);
	reg_bin_key(K3DS_B, KEY_B);
	reg_bin_key(K3DS_X, KEY_X);
	reg_bin_key(K3DS_Y, KEY_Y);

	reg_bin_key(K3DS_L, KEY_L);
	reg_bin_key(K3DS_R, KEY_R);

	reg_bin_key(K3DS_START, KEY_START);
	reg_bin_key(K3DS_SELECT, KEY_SELECT);

	reg_bin_key(K3DS_DUP, KEY_DUP);
	reg_bin_key(K3DS_DDOWN, KEY_DDOWN);
	reg_bin_key(K3DS_DLEFT, KEY_DLEFT);
	reg_bin_key(K3DS_DRIGHT, KEY_DRIGHT);

	float circX = (float)input->circlePadX / 0x9c;
	float circY = (float)input->circlePadY / 0x9c;

#define reg_cpad_key(i, c, k)                                                          \
	ctrls[(i)] = ABS(c) * (float)((input->keysdown & k) || (input->keysheld & k)); \
	keysdown[(i)] = input->keysdown & (k);                                         \
	keysup[(i)] = input->keysup & (k)

	reg_cpad_key(K3DS_CPAD_UP, circY, KEY_CPAD_UP);
	reg_cpad_key(K3DS_CPAD_DOWN, circY, KEY_CPAD_DOWN);
	reg_cpad_key(K3DS_CPAD_LEFT, circX, KEY_CPAD_LEFT);
	reg_cpad_key(K3DS_CPAD_RIGHT, circX, KEY_CPAD_RIGHT);
#undef reg_bin_key
}
#else
#error "Input conversion functions not implemented for this platform"
#endif

typedef struct {
	float keys[PLATFORM_BUTTONS];
	bool keysup[PLATFORM_BUTTONS];
	bool keysdown[PLATFORM_BUTTONS];
	int layer;
} PlatformAgnosticInput;

static inline float IsKeyDown(KeyCombo combo, PlatformAgnosticInput* input) { return input->keys[combo]; }
static inline bool WasKeyReleased(KeyCombo combo, PlatformAgnosticInput* input) { return input->keysup[combo]; }

void PlayerController_Init(PlayerController* ctrl, Player* player) {
	ctrl->breakPlaceTimeout = 0.f;
	ctrl->player = player;
	ctrl->controlScheme = platform_default_scheme;
	// TODO: alternative Steuerung aus Datei laden 2 %
}

void PlayerController_Update(PlayerController* ctrl, InputData input, float dt) {
	Player* player = ctrl->player;
	PlatformAgnosticInput agnosticInput;
	convertPlatformInput(&input, agnosticInput.keys, agnosticInput.keysdown, agnosticInput.keysup);

	float forward = IsKeyDown(ctrl->controlScheme.forward, &agnosticInput);
	float backward = IsKeyDown(ctrl->controlScheme.backward, &agnosticInput);
	float strafeLeft = IsKeyDown(ctrl->controlScheme.strafeLeft, &agnosticInput);
	float strafeRight = IsKeyDown(ctrl->controlScheme.strafeRight, &agnosticInput);

	float3 forwardVec = f3_new(-sinf(player->yaw), 0.f, -cosf(player->yaw));
	float3 rightVec = f3_crs(forwardVec, f3_new(0, 1, 0));

	float3 movement = f3_new(0, 0, 0);
	movement = f3_add(movement, f3_scl(forwardVec, forward));
	movement = f3_sub(movement, f3_scl(forwardVec, backward));
	movement = f3_add(movement, f3_scl(rightVec, strafeRight));
	movement = f3_sub(movement, f3_scl(rightVec, strafeLeft));
	if (f3_magSqr(movement) > 0.f) {
		float speed = 4.3f * f3_mag(f3_new(-strafeLeft + strafeRight, 0, -forward + backward));
		player->bobbing += speed * 1.5f * dt;
		movement = f3_scl(f3_nrm(movement), speed);
	}

	float lookLeft = IsKeyDown(ctrl->controlScheme.lookLeft, &agnosticInput);
	float lookRight = IsKeyDown(ctrl->controlScheme.lookRight, &agnosticInput);
	float lookUp = IsKeyDown(ctrl->controlScheme.lookUp, &agnosticInput);
	float lookDown = IsKeyDown(ctrl->controlScheme.lookDown, &agnosticInput);

	player->yaw += (lookLeft + -lookRight) * 160.f * DEG_TO_RAD * dt;
	player->pitch += (-lookDown + lookUp) * 160.f * DEG_TO_RAD * dt;
	player->pitch = CLAMP(player->pitch, -DEG_TO_RAD * 89.9f, DEG_TO_RAD * 89.9f);

	float placeBlock = IsKeyDown(ctrl->controlScheme.placeBlock, &agnosticInput);
	float breakBlock = IsKeyDown(ctrl->controlScheme.breakBlock, &agnosticInput);
	if (placeBlock > 0.f) Player_PlaceBlock(player, player->blockInHand);
	if (breakBlock > 0.f) Player_BreakBlock(player);

	float jump = IsKeyDown(ctrl->controlScheme.jump, &agnosticInput);
	if (jump > 0.f) Player_Jump(player, movement);

	bool switchBlockLeft = WasKeyReleased(ctrl->controlScheme.switchBlockLeft, &agnosticInput);
	bool switchBlockRight = WasKeyReleased(ctrl->controlScheme.switchBlockRight, &agnosticInput);
	if (switchBlockLeft && --player->blockInHand == 0) player->blockInHand = Blocks_Count - 1;
	if (switchBlockRight && ++player->blockInHand == Blocks_Count) player->blockInHand = 1;

	DebugUI_Text("%s", BlockNames[player->blockInHand]);

	Player_Move(player, dt, movement);
	Player_Update(player);
}