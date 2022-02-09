//
// Created by Joe Chrisman on 2/8/22.
//

#include "Board.h"

Board::Board()
{
    // initialize board with 8 null elements
    board = std::vector<std::vector<Square*>>(8);

    whitePieces = std::vector<Piece*>();
    blackPieces = std::vector<Piece*>();

    int whitePawnRow = ENGINE_IS_BLACK ? 6 : 1;
    int whitePieceRow = ENGINE_IS_BLACK ? 7 : 0;
    int blackPawnRow = ENGINE_IS_BLACK ? 1 : 6;
    int blackPieceRow = ENGINE_IS_BLACK ? 0 : 7;

    for (int row = 0; row < 8; row++)
    {
        // initialize rank with 8 null elements
        auto rank = std::vector<Square*>(8);
        for (int col = 0; col < 8; col++)
        {
            // create a square and add it to the rank
            Square *square = new Square(row, col);
            rank[col] = square;

            // we are going to potentially add a piece
            Piece *addedPiece = nullptr;
            PIECE_TYPE type = ENGINE_IS_BLACK ? PIECE_LAYOUT_WHITE_BOTTOM[col] : PIECE_LAYOUT_BLACK_BOTTOM[col];

            if (row == blackPawnRow)
            {
                // create black pawns
                addedPiece = new Piece(false, PAWN, row, col);
            }
            else if (row == whitePawnRow)
            {
                // create white pawns
                addedPiece = new Piece(true, PAWN, row, col);
            }
            else if (row == blackPieceRow)
            {
                // create black pieces
                addedPiece = new Piece(false, type, row, col);
            }
            else if (row == whitePieceRow)
            {
                // create white pieces
                addedPiece = new Piece(true, type, row, col);
            }

            // if there should be a piece on this square
            if (addedPiece != nullptr)
            {
                // add the piece to this square
                square -> piece = addedPiece;
                if (addedPiece -> isWhite)
                {
                    // add the white piece to white pieces
                    whitePieces.push_back(addedPiece);
                    if (addedPiece -> type == KING)
                    {
                        // if we added a king remember it
                        whiteKing = addedPiece;
                    }
                }
                else
                {
                    // add the black piece to the black pieces
                    blackPieces.push_back(addedPiece);
                    if (addedPiece -> type == KING)
                    {
                        // if we added a king remember it
                        blackKing = addedPiece;
                    }
                }
            }
        }
        // add the rank to the board
        board[row] = rank;
    }
}

// render the board
void Board::render(SDL_Renderer *renderer)
{
    for (std::vector<Square*> &rank : board)
    {
        for (Square* &square : rank)
        {
            // squares render pieces if they have them
            square -> render(renderer);
        }
    }

}
