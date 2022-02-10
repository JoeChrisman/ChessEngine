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

    // keep track of last piece user clicked
    Piece *pieceClicked;
    // keep track of last piece moved for en passant
    Piece *lastMoved;

    void render(SDL_Renderer *renderer);
    void onClicked(int x, int y);
    void resetHighlights();
    bool isOnBoard(int row, int col);
    void movePiece(Piece *moving, Square *targetSquare, std::vector<Piece*> &piecesRemoved);
    void removePiece(Piece *toRemove);
    // make a random move
    void makeBestMove();
    std::vector<Square*> getMoves(Piece *moving);

};


#endif //ENGINE2_BOARD_H
