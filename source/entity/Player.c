#include <entity/Player.h>

#include <misc/Collision.h>

void Player_Init(Player* player) {
	player->position = f3_new(0.f, 0.f, 0.f);

	player->bobbing = 0.f;
	player->pitch = 0.f;
	player->yaw = 0.f;

	player->grounded = false;
	player->sprinting = false;
	player->world = NULL;

	player->fovAdd = 0.f;
	player->crouchAdd = 0.f;

	player->view = f3_new(0, 0, -1);

	player->crouching = false;
	player->flying = false;

	player->blockInSeight = false;
	player->blockInActionRange = false;

	player->velocity = f3_new(0, 0, 0);
	player->simStepAccum = 0.f;

	player->breakPlaceTimeout = 0.f;

	player->debugButton = false;
}

void Player_Spawn(Player* player, World* world) { player->world = world; }

void Player_Update(Player* player) {
	player->view = f3_new(-sinf(player->yaw) * cosf(player->pitch), sinf(player->pitch), -cosf(player->yaw) * cosf(player->pitch));

	player->blockInSeight = Raycast_Cast(player->world, f3_new(player->position.x, player->position.y + PLAYER_EYEHEIGHT, player->position.z), player->view,
					     &player->viewRayCast);
	player->blockInActionRange = player->blockInSeight && player->viewRayCast.distSqr < 5.f * 5.f * 5.f;
}

bool Player_CanMove(Player* player, float newX, float newY, float newZ) {
	for (int x = -1; x < 2; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = -1; z < 2; z++) {
				int pX = FastFloor(newX) + x;
				int pY = FastFloor(newY) + y;
				int pZ = FastFloor(newZ) + z;
				if (World_GetBlock(player->world, pX, pY, pZ) != Block_Air) {
					if (AABB_Overlap(newX - PLAYER_COLLISIONBOX_SIZE / 2.f, newY, newZ - PLAYER_COLLISIONBOX_SIZE / 2.f,
							 PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT, PLAYER_COLLISIONBOX_SIZE, pX, pY, pZ, 1.f, 1.f, 1.f)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

void Player_Jump(Player* player, float3 accl) {
	if (player->grounded) {
		player->velocity.x = accl.x * 1.1f;
		player->velocity.z = accl.z * 1.1f;
		player->velocity.y = 6.7f;
		player->jumped = true;
	}
}
#include <gui/DebugUI.h>
const float MaxWalkVelocity = 4.3f;
const float MaxFallVelocity = -50.f;
const float GravityPlusFriction = 10.f;
void Player_Move(Player* player, float dt, float3 accl) {
	player->breakPlaceTimeout -= dt;
	player->simStepAccum += dt;
	const float SimStep = 1.f / 60.f;
	while (player->simStepAccum >= SimStep) {
		player->velocity.y -= GravityPlusFriction * SimStep * 2.f;
		float speedFactor = 1.f;
		if (!player->grounded) {
			if (player->jumped)
				speedFactor = 0.2f;
			else
				speedFactor = 0.6f;
		}
		float3 newPos = f3_add(player->position, f3_add(f3_scl(player->velocity, SimStep), f3_scl(accl, SimStep * speedFactor)));
		float3 finalPos = player->position;

		player->grounded = false;

		bool wallCollision = false;

		for (int i = 0; i < 3; i++) {
			bool collision = false;
			float3 axisStep =
			    f3_new(i == 0 ? newPos.x : player->position.x, i == 1 ? newPos.y : player->position.y, i == 2 ? newPos.z : player->position.z);
			Box playerBox = Box_Create(axisStep.x - PLAYER_COLLISIONBOX_SIZE / 2.f, axisStep.y, axisStep.z - PLAYER_COLLISIONBOX_SIZE / 2.f,
						   PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT, PLAYER_COLLISIONBOX_SIZE);

			for (int x = -1; x < 2; x++) {
				for (int y = 0; y < 3; y++) {
					for (int z = -1; z < 2; z++) {
						int pX = FastFloor(axisStep.x) + x;
						int pY = FastFloor(axisStep.y) + y;
						int pZ = FastFloor(axisStep.z) + z;
						if (World_GetBlock(player->world, pX, pY, pZ) != Block_Air) {
							Box blockBox = Box_Create(pX, pY, pZ, 1, 1, 1);

							float3 normal = f3_new(0.f, 0.f, 0.f);
							float depth = 0.f;
							int face = 0;

							bool intersects = Collision_BoxIntersect(blockBox, playerBox, &normal, &depth, &face);
							collision |= intersects;
						}
					}
				}
			}

			if (!collision)
				finalPos.v[i] = newPos.v[i];
			else if (i == 1) {
				if (player->velocity.y < 0.f) player->grounded = true;
				player->jumped = false;
				player->velocity.x = 0.f;
				player->velocity.y = 0.f;
				player->velocity.z = 0.f;
			} else {
				wallCollision = true;
				if (i == 0)
					player->velocity.x = 0.f;
				else
					player->velocity.z = 0.f;
			}
		}

		if (wallCollision) Player_Jump(player, accl);

		player->position = finalPos;
		player->velocity = f3_new(player->velocity.x * 0.95f, player->velocity.y, player->velocity.z * 0.95f);
		if (ABS(player->velocity.x) < 0.1f) player->velocity.x = 0.f;
		if (ABS(player->velocity.z) < 0.1f) player->velocity.z = 0.f;

		player->simStepAccum -= SimStep;
	}
}

static const float BreakPlace_Timout = 0.2f;
void Player_PlaceBlock(Player* player, Block block) {
	if (player->world && player->blockInActionRange && player->breakPlaceTimeout < 0.f) {
		const int* offset = DirectionToOffset[player->viewRayCast.direction];
		if (AABB_Overlap(player->position.x - PLAYER_COLLISIONBOX_SIZE / 2.f, player->position.y, player->position.z - PLAYER_COLLISIONBOX_SIZE / 2.f,
				 PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT, PLAYER_COLLISIONBOX_SIZE, player->viewRayCast.x + offset[0],
				 player->viewRayCast.y + offset[1], player->viewRayCast.z + offset[2], 1.f, 1.f, 1.f))
			return;
		World_SetBlock(player->world, player->viewRayCast.x + offset[0], player->viewRayCast.y + offset[1], player->viewRayCast.z + offset[2], block);
		player->breakPlaceTimeout = BreakPlace_Timout;
	}
}

void Player_BreakBlock(Player* player) {
	if (player->world && player->blockInActionRange && player->breakPlaceTimeout < 0.f) {
		World_SetBlock(player->world, player->viewRayCast.x, player->viewRayCast.y, player->viewRayCast.z, Block_Air);
		player->breakPlaceTimeout = BreakPlace_Timout;
	}
}