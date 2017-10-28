#pragma once

#include <stdbool.h>

#include <blocks/Block.h>

#include <misc/NumberUtils.h>

typedef struct {
	Block block;
	uint8_t meta, amount;
} ItemStack;

#define ITEMSTACK_MAX (64)

inline bool ItemStack_Empty(ItemStack stack) { return stack.amount == 0; }
inline void ItemStack_Transfer(ItemStack* src, ItemStack* dst) {
	if ((src->block == dst->block && src->meta == dst->meta) || dst->amount == 0) {
		int vol = MIN(src->amount, ITEMSTACK_MAX - dst->amount);
		src->amount -= vol;
		dst->amount += vol;
		dst->block = src->block;
		dst->meta = src->meta;
	} else {
		ItemStack tmp = *src;
		*src = *dst;
		*dst = tmp;
	}
}