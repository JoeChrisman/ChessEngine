//
// Created by Joe Chrisman on 2/9/22.
//

#ifndef ENGINE2_SQUARE_H
#define ENGINE2_SQUARE_H

#include "Piece.h"

class Square {
public:
    int row;
    int col;

    Piece *piece;
    SDL_Rect *bounds;
    bool isHighlighted;

    Square(int row, int col);
    void render(SDL_Renderer *renderer);
};


#endif //ENGINE2_SQUARE_H
