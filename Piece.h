//
// Created by Joe Chrisman on 2/9/22.
//

#ifndef ENGINE2_PIECE_H
#define ENGINE2_PIECE_H

#include "Constants.h"

class Piece {
public:
    int row;
    int col;
    int timesMoved;
    bool isWhite;
    PIECE_TYPE type;

    Piece(bool isWhite, PIECE_TYPE type, int row, int col);
    void render(SDL_Renderer *renderer, SDL_Rect *bounds);
    std::string getImagePath();
};


#endif //ENGINE2_PIECE_H
