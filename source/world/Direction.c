#include <world/Direction.h>

int DirectionToOffset[7][3] = {{-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}, {0, 0, 0}};
Direction DirectionOpposite[7] = {Direction_East, Direction_West, Direction_Top, Direction_Bottom, Direction_South, Direction_North, Direction_Invalid};
Axis DirectionToAxis[7] = {Axis_X, Axis_X, Axis_Y, Axis_Y, Axis_Z, Axis_Z, Axis_X};