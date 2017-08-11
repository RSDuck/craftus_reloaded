#pragma once

typedef enum {
    Direction_West,
    Direction_East,
    Direction_Bottom,
    Direction_Top,
    Direction_North,
    Direction_South,
    Direction_Invalid
} Direction;

typedef enum {
    Axis_X,
    Axis_Y,
    Axis_Z
}Axis;

extern int DirectionToOffset[7][3];
extern Direction DirectionOpposite[7];
extern Axis DirectionToAxis[7];