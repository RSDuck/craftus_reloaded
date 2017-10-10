#pragma once

#include <stdbool.h>

#include <blocks/Block.h>

typedef struct {
	Block block;
	uint8_t meta, amount;
} ItemStack;

#define ITEMSTACK_MAX (64)

inline bool ItemStack_Empty(ItemStack stack) { return stack.amount == 0; }