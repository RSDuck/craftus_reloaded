#include <entity/Player.h>

#include <misc/Collision.h>

void Player_Init(Player* player, World* world) {
	player->position = f3_new(0.f, 0.f, 0.f);

	player->bobbing = 0.f;
	player->pitch = 0.f;
	player->yaw = 0.f;

	player->grounded = false;
	player->sprinting = false;
	player->world = world;

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

	player->quickSelectBarSlots = INVENTORY_QUICKSELECT_MAXSLOTS;
	player->quickSelectBarSlot = 0;
	{
		int l = 0;
		player->inventory[l++] = (ItemStack){Block_Stone, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Dirt, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Grass, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Cobblestone, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Sand, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Log, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Leaves, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Glass, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Stonebrick, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Brick, 0, 1};
		player->inventory[l++] = (ItemStack){Block_Planks, 0, 1};
		for (int i = 0; i < 16; i++) player->inventory[l++] = (ItemStack){Block_Wool, i, 1};
		player->inventory[l++] = (ItemStack){Block_Bedrock, 0, 1};

		for (int i = 0; i < INVENTORY_QUICKSELECT_MAXSLOTS; i++) player->quickSelectBar[i] = (ItemStack){Block_Air, 0, 0};
	}

	player->autoJumpEnabled = true;
}

void Player_Update(Player* player) {
	player->view = f3_new(-sinf(player->yaw) * cosf(player->pitch), sinf(player->pitch), -cosf(player->yaw) * cosf(player->pitch));

	player->blockInSeight =
	    Raycast_Cast(player->world, f3_new(player->position.x, player->position.y + PLAYER_EYEHEIGHT, player->position.z), player->view,
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
							 PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT, PLAYER_COLLISIONBOX_SIZE, pX, pY, pZ, 1.f,
							 1.f, 1.f)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

void Player_Jump(Player* player, float3 accl) {
	if (player->grounded && !player->flying) {
		player->velocity.x = accl.x * 1.1f;
		player->velocity.z = accl.z * 1.1f;
		player->velocity.y = 6.7f;
		player->jumped = true;
		player->crouching = false;
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
		if (player->velocity.y < MaxFallVelocity) player->velocity.y = MaxFallVelocity;

		if (player->flying) player->velocity.y = 0.f;

		float speedFactor = 1.f;
		if (!player->grounded && !player->flying) {
			if (player->jumped)
				speedFactor = 0.2f;
			else
				speedFactor = 0.6f;
		} else if (player->flying)
			speedFactor = 2.f;
		else if (player->crouching)
			speedFactor = 0.5f;
		float3 newPos = f3_add(player->position, f3_add(f3_scl(player->velocity, SimStep), f3_scl(accl, SimStep * speedFactor)));
		float3 finalPos = player->position;

		bool wallCollision = false, wasGrounded = player->grounded;

		player->grounded = false;
		for (int j = 0; j < 3; j++) {
			int i = (int[]){0, 2, 1}[j];
			bool collision = false;
			float3 axisStep = /*f3_new(i == 0 ? newPos.x : player->position.x, i == 1 ? newPos.y : player->position.y,
						 i == 2 ? newPos.z : player->position.z)*/ finalPos;
			axisStep.v[i] = newPos.v[i];
			Box playerBox =
			    Box_Create(axisStep.x - PLAYER_COLLISIONBOX_SIZE / 2.f, axisStep.y, axisStep.z - PLAYER_COLLISIONBOX_SIZE / 2.f,
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

							bool intersects =
							    Collision_BoxIntersect(blockBox, playerBox, 0, &normal, &depth, &face);
							collision |= intersects;
						}
					}
				}
			}
			if (!collision)
				finalPos.v[i] = newPos.v[i];
			else if (i == 1) {
				if (player->velocity.y < 0.f || accl.y < 0.f) player->grounded = true;
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

		float3 movDiff = f3_sub(finalPos, player->position);

		if (player->grounded && player->flying) player->flying = false;

		if (wallCollision && player->autoJumpEnabled) {
			float3 nrmDiff = f3_nrm(f3_sub(newPos, player->position));
			Block block = World_GetBlock(player->world, FastFloor(finalPos.x + nrmDiff.x),
						     FastFloor(finalPos.y + nrmDiff.y) + 2, FastFloor(finalPos.z + nrmDiff.z));
			Block landingBlock = World_GetBlock(player->world, FastFloor(finalPos.x + nrmDiff.x),
							    FastFloor(finalPos.y + nrmDiff.y) + 1, FastFloor(finalPos.z + nrmDiff.z));
			if (block == Block_Air && landingBlock != Block_Air) Player_Jump(player, accl);
		}

		if (player->crouching && player->crouchAdd > -0.3f) player->crouchAdd -= SimStep * 2.f;
		if (!player->crouching && player->crouchAdd < 0.0f) player->crouchAdd += SimStep * 2.f;

		if (player->crouching && !player->grounded && wasGrounded && finalPos.y < player->position.y && movDiff.x != 0.f &&
		    movDiff.z != 0.f) {
			finalPos = player->position;
			player->grounded = true;
			player->velocity.y = 0.f;
		}

		player->position = finalPos;
		player->velocity = f3_new(player->velocity.x * 0.95f, player->velocity.y, player->velocity.z * 0.95f);
		if (ABS(player->velocity.x) < 0.1f) player->velocity.x = 0.f;
		if (ABS(player->velocity.z) < 0.1f) player->velocity.z = 0.f;

		player->simStepAccum -= SimStep;
	}
}

void Player_PlaceBlock(Player* player) {
	if (player->world && player->blockInActionRange && player->breakPlaceTimeout < 0.f) {
		const int* offset = DirectionToOffset[player->viewRayCast.direction];
		if (AABB_Overlap(player->position.x - PLAYER_COLLISIONBOX_SIZE / 2.f, player->position.y,
				 player->position.z - PLAYER_COLLISIONBOX_SIZE / 2.f, PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT,
				 PLAYER_COLLISIONBOX_SIZE, player->viewRayCast.x + offset[0], player->viewRayCast.y + offset[1],
				 player->viewRayCast.z + offset[2], 1.f, 1.f, 1.f))
			return;
		World_SetBlockAndMeta(player->world, player->viewRayCast.x + offset[0], player->viewRayCast.y + offset[1],
				      player->viewRayCast.z + offset[2], player->quickSelectBar[player->quickSelectBarSlot].block,
				      player->quickSelectBar[player->quickSelectBarSlot].meta);
	}
	if (player->breakPlaceTimeout < 0.f) player->breakPlaceTimeout = PLAYER_PLACE_REPLACE_TIMEOUT;
}

void Player_BreakBlock(Player* player) {
	if (player->world && player->blockInActionRange && player->breakPlaceTimeout < 0.f) {
		World_SetBlock(player->world, player->viewRayCast.x, player->viewRayCast.y, player->viewRayCast.z, Block_Air);
	}
	if (player->breakPlaceTimeout < 0.f) player->breakPlaceTimeout = PLAYER_PLACE_REPLACE_TIMEOUT;
}

void Player_Teleport(Player* player, float x, float y, float z) {
	player->position.x = x;
	player->position.y = y;
	player->position.z = z;

	player->velocity = f3_new(0, 0, 0);
	Player_Update(player);
}