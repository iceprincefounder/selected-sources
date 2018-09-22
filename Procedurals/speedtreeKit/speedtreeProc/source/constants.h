#pragma once

#include <cstring>

#define SPEEDTREE_BRANCHES 0
#define SPEEDTREE_FRONDANDCAPS 1
#define SPEEDTREE_LEAVES 2
#define SPEEDTREE_FACINGLEAVES 3
#define SPEEDTREE_RIGIDMESHES 4
#define SPEEDTREE_GRASS 5
#define SPEEDTREE_BILLBOARD 6


enum SpeedTreePoly
{
    ST_BRANCHES = 0,
    ST_FRONDANDCAPS,
    ST_LEAVES,
    ST_FACINGLEAVES,
    ST_RIGIDMESHES,
    ST_GRASS,
    ST_BILLBOARD,
};

#define SPEEDTREE_POLY_COUNT 4

#define SPEEDTREE_SHADER "TheSpeedtreeShader"