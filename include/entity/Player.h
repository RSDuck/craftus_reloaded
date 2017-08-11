#pragma once

#include <stdbool.h>

#include <world/World.h>

#include <misc/Raycast.h>
#include <misc/VecMath.h>

#define PLAYER_EYEHEIGHT (1.7f)
#define PLAYER_HEIGHT (1.8f)
#define PLAYER_COLLISIONBOX_SIZE (0.6f)

typedef struct {
	float3 position;
	float pitch, yaw;
	float bobbing, fovAdd, crouchAdd;
	bool grounded, jumped, sprinting, flying, crouching;
	World* world;

	float3 view;

	float3 velocity;
	float simStepAccum;

	float breakPlaceTimeout;

	Block blockInHand;

	Raycast_Result viewRayCast;
	bool blockInSeight, blockInActionRange;
} Player;

void Player_Init(Player* player);
void Player_Spawn(Player* player, World* world);

void Player_Update(Player* player);

void Player_Move(Player* player, float dt, float3 accl);

void Player_PlaceBlock(Player* player, Block block);
void Player_BreakBlock(Player* player);

void Player_Jump(Player* player, float3 accl);