#include <world/Chunk.h>

Xorshift32 uuidGenerator = (Xorshift32)42434243;

const uint8_t _seethroughTable[6][6] = {
    // W E B T N S
    {255, 0, 1, 3, 6, 10},     // West
    {0, 255, 2, 4, 7, 11},     // East
    {1, 2, 255, 5, 8, 12},     // Bottom
    {3, 4, 5, 255, 9, 13},     // Top
    {6, 7, 8, 9, 255, 14},     // North
    {10, 11, 12, 13, 14, 255}  // South
};
