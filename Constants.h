//
// Created by Joe Chrisman on 2/8/22.
//

#ifndef ENGINE2_CONSTANTS_H
#define ENGINE2_CONSTANTS_H

#include <vector>
#include <string>
#include "SDL2/SDL.h"

const bool ENGINE_IS_BLACK = true;

enum PIECE_TYPE
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

const PIECE_TYPE PIECE_LAYOUT_WHITE_BOTTOM[] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
const PIECE_TYPE PIECE_LAYOUT_BLACK_BOTTOM[] = {ROOK, KNIGHT, BISHOP, KING, QUEEN, BISHOP, KNIGHT, ROOK};

const int WINDOW_SIZE = 800;
const int SQUARE_SIZE = WINDOW_SIZE / 8;

// dark square color
const int DARK_R = 0;
const int DARK_G = 0;
const int DARK_B = 0;

// light square color
const int LIGHT_R = 200;
const int LIGHT_G = 200;
const int LIGHT_B = 200;

// highlighted square color
const int HIGHLIGHT_R = 60;
const int HIGHLIGHT_G = 250;
const int HIGHLIGHT_B = 100;


#endif //ENGINE2_CONSTANTS_H
