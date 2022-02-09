//
// Created by Joe Chrisman on 2/8/22.
//

#ifndef ENGINE2_BOARD_H
#define ENGINE2_BOARD_H

#include "Square.h"

class Board {
public:
    Board();

    // a 2d vector of square pointers will serve as the board
    std::vector<std::vector<Square*>> board;
    // keep track of pieces on the board by color
    std::vector<Piece*> whitePieces;
    std::vector<Piece*> blackPieces;

    // keep track of both kings
    Piece *whiteKing;
    Piece *blackKing;

    void render(SDL_Renderer *renderer);

};


#endif //ENGINE2_BOARD_H
