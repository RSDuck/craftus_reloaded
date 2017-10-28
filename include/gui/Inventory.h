#pragma once

#include <inventory/ItemStack.h>

#include <misc/NumberUtils.h>

#define INVENTORY_QUICKSELECT_MAXSLOTS 9
#define INVENTORY_QUICKSELECT_HEIGHT (22 + 1) // + 1 wegen dem Selector

inline int Inventory_QuickSelectCalcSlots(int screenwidth) { return MIN(INVENTORY_QUICKSELECT_MAXSLOTS, (screenwidth - 21 * 2) / 20 + 2); }
inline int Inventory_QuickSelectCalcWidth(int slots) { return 42 + (slots - 2) * 20; }

void Inventory_DrawQuickSelect(int x, int y, ItemStack* stacks, int count, int* selected);

void Inventory_Draw(int x, int y, int w, ItemStack* stacks, int count);