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
    for (auto &rank : board)
    {
        for (Square* &square : rank)
        {
            // squares render pieces if they have them
            square -> render(renderer);
        }
    }
}

Piece* Board::onClicked(int x, int y)
{
    SDL_Point *mouse = new SDL_Point{x, y};
    for (auto &rank : board)
    {
        for (Square* &square : rank)
        {
            if (SDL_PointInRect(mouse, square -> bounds))
            {
                resetHighlights();
                Piece *piece = square -> piece;
                if (piece != nullptr && piece -> isWhite == ENGINE_IS_BLACK)
                {
                    // highlight legal moves
                    std::vector<Square*> moves = getMoves(piece);
                    for (Square* &move : moves)
                    {
                        move -> isHighlighted = true;
                    }
                }
                if (square -> isHighlighted)
                {
                    // movePiece(piece, square);
                }
                return piece;
            }
        }
    }
}

void Board::resetHighlights()
{
    for (auto &rank : board)
    {
        for (Square* &square : rank)
        {
            square -> isHighlighted = false;
        }
    }
}

bool Board::isOnBoard(int row, int col)
{
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

// get raw moves for a piece
// these types of moves don't account for check-related rules
std::vector<Square*> Board::getMoves(Piece *moving)
{
    int row = moving -> row;
    int col = moving -> col;
    PIECE_TYPE type = moving -> type;
    bool isWhite = moving -> isWhite;

    std::vector<Square*> moves;

    if (type == PAWN)
    {
        int direction = ENGINE_IS_BLACK ? -1 : 1;
        // check for the pawn moving one square
        Square *oneSquareAhead = board[row + direction][col];
        if (oneSquareAhead -> piece == nullptr)
        {
            moves.push_back(oneSquareAhead);
            if (moving -> timesMoved == 0)
            {
                // check for the pawn moving two squares
                Square *twoSquaresAhead = board[row + direction * 2][col];
                if (twoSquaresAhead -> piece == nullptr)
                {
                    moves.push_back(twoSquaresAhead);
                }
            }
        }
        // don't go off the edge of the board
        if (col > 0)
        {
            // check for left capture
            Square *leftCapture = board[row + direction][col - 1];
            if (leftCapture -> piece != nullptr && leftCapture -> piece -> isWhite != isWhite)
            {
                moves.push_back(leftCapture);
            }
        }
        // don't go off the edge of the board
        if (col < 7)
        {
            // check for right capture
            Square *rightCapture = board[row + direction][col + 1];
            if (rightCapture -> piece != nullptr && rightCapture -> piece -> isWhite != isWhite)
            {
                moves.push_back(rightCapture);
            }
        }

    }
    else if (type == KNIGHT)
    {
        // offsets for how a knight moves
        int offsets[8][2] = {{-2, 1}, {-2, -1}, {2, -1}, {2, 1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

        // go through the offsets
        for (int moveIndex = 0; moveIndex < 8; moveIndex++)
        {
            // figure out what square we landed on
            int r = row + offsets[moveIndex][0];
            int c = col + offsets[moveIndex][1];
            if (isOnBoard(r, c))
            {
                Square *move = board[r][c];
                // if our landing square is empty or is a capture square
                if (move -> piece == nullptr || move -> piece -> isWhite != ENGINE_IS_BLACK)
                {
                    moves.push_back(board[r][c]);
                }
            }

        }

    }
    else if (type == KING)
    {
        // offset king's x
        for (int dx = -1; dx <= 1; dx++)
        {
            // offset king's y
            for (int dy = -1; dy <= 1; dy++)
            {
                // if we are moving somewhere
                if (dx != 0 || dy != 0)
                {
                    // figure out where we moved to
                    int x = col + dx;
                    int y = row + dy;
                    // make sure we dont go off the board
                    if (isOnBoard(y, x))
                    {
                        Square *move = board[y][x];
                        // if the square is empty or is a capture square
                        if (move -> piece == nullptr || move -> piece -> isWhite != isWhite) {
                            moves.push_back(move);
                        }
                    }
                }
            }
        }
    }
    else if (type == BISHOP || type == QUEEN)
    {
        // check each diagonal
        for (int direction = 0; direction < 4; direction++)
        {
            // check along the path of the diagonal
            for (int distance = 1; distance < 8; distance++)
            {
                // figure out where our diagonal move is based on direction and distance
                int r = direction < 2 ? row + distance : row - distance;
                int c = (direction == 0 || direction == 3 ? col + distance : col - distance);
                // make sure we don't go off the board
                if (!isOnBoard(r, c))
                {
                    break;
                }
                Square *move = board[r][c];
                // if the destination square was empty or a capture square
                if (move -> piece == nullptr || move -> piece -> isWhite != isWhite)
                {
                    // add it as a move
                    moves.push_back(move);
                }
                else
                {
                    break;
                }
            }
        }

    }
    if (type == ROOK || type == QUEEN)
    {
        // check each cardinal direction
        for (int direction = 0; direction < 4; direction++)
        {
            // check along that direction
            for (int distance = 1; distance < 8; distance++)
            {
                int x = 0;
                int y = 0;
                // figure out our destination square by direction and distance
                if (direction < 2)
                {
                    x = direction == 1 ? distance : -distance;
                }
                else
                {
                    y = direction == 2 ? distance : -distance;
                }
                x += col;
                y += row;
                // make sure to not go off the board
                if (!isOnBoard(y, x)) {
                    break;
                }
                Square *move = board[y][x];
                // if the destination square is empty or is a capture square
                if (move -> piece == nullptr || move -> piece -> isWhite != isWhite)
                {
                    moves.push_back(move);
                }
                else
                {
                    break;
                }
            }
        }
    }

    return moves;

}
