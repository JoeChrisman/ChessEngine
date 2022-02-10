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

    pieceClicked = nullptr;
    lastMoved = nullptr;

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
                        // if we added a white king remember it
                        whiteKing = addedPiece;
                    }
                }
                else
                {
                    // add the black piece to the black pieces
                    blackPieces.push_back(addedPiece);
                    if (addedPiece -> type == KING)
                    {
                        // if we added a black king remember it
                        blackKing = addedPiece;
                    }
                }
            }
        }
        // add the rank to the board
        board[row] = rank;
    }

    if (!ENGINE_IS_BLACK)
    {
        makeBestMove();
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

void Board::removePiece(Piece *toRemove)
{
    // get a pointer to whatever piece color vector we need
    std::vector<Piece*> *pieces = toRemove -> isWhite ? &whitePieces : &blackPieces;
    // remove piece from the vector
    auto removalIndex = std::find(pieces -> begin(), pieces -> end(), toRemove);
    if (removalIndex != pieces -> end())
    {
        pieces -> erase(removalIndex);
    }
}

void Board::movePiece(Piece *moving, Square *targetSquare, std::vector<Piece*> &piecesRemoved)
{
    int rowFrom = moving -> row;
    int colFrom = moving -> col;
    int rowTo = targetSquare -> row;
    int colTo = targetSquare -> col;
    PIECE_TYPE type = moving -> type;

    // remove piece we are moving from the board
    board[rowFrom][colFrom] -> piece = nullptr;

    Piece *captured = targetSquare -> piece;
    // if we captured a piece
    if (captured != nullptr)
    {
        // remember pieces we remove
        piecesRemoved.push_back(captured);
        // remove captured piece
        removePiece(captured);
    }
    // if we are moving a pawn
    if (type == PAWN)
    {
        // if we captured a pawn en passant
        if (targetSquare -> col != moving -> col && captured == nullptr)
        {
            // remember en passant captured pawn and remove it from the board
            captured = board[rowFrom][colTo] -> piece;
            board[rowFrom][colTo] -> piece = nullptr;
            // remember pieces we remove
            piecesRemoved.push_back(captured);
            // remove en passant captured pawn
            removePiece(captured);
        }
        // if we are promoting a pawn
        if (rowTo == 0 || rowTo == 7)
        {
            // remember pieces we remove
            piecesRemoved.push_back(moving);
            // remove pawn we promoted
            removePiece(moving);
            // promote to a queen
            moving = new Piece(moving -> isWhite, QUEEN, rowTo, colTo);
            // offset for when we actually move the promoted queen to the square
            moving -> timesMoved = -1;
            // add promoted queen
            if (moving -> isWhite)
            {
                whitePieces.push_back(moving);
            }
            else
            {
                blackPieces.push_back(moving);
            }
        }
    }
    // if we are castling
    if (type == KING && abs(colFrom - colTo) > 1)
    {
        // if we are castling left
        if (colTo < 4)
        {
            // move left rook to right of king
            Piece *leftRook = board[rowTo][0] -> piece;
            leftRook -> col = colTo + 1;
            leftRook -> timesMoved++;
            board[rowTo][colTo + 1] -> piece = leftRook;
            board[rowTo][0] -> piece = nullptr;
        }
        // if we are castling right
        else
        {
            // move right rook to left of king
            Piece *rightRook = board[rowTo][7] -> piece;
            rightRook -> col = colTo - 1;
            rightRook -> timesMoved++;
            board[rowTo][colTo - 1] -> piece = rightRook;
            board[rowTo][7] -> piece = nullptr;
        }
    }

    moving -> row = rowTo;
    moving -> col = colTo;
    moving -> timesMoved++;
    targetSquare -> piece = moving;

    lastMoved = moving;
}

void Board::onClicked(int x, int y)
{
    SDL_Point *mouse = new SDL_Point{x, y};
    for (auto &rank : board)
    {
        for (Square* &square : rank)
        {
            if (SDL_PointInRect(mouse, square -> bounds))
            {
                Piece *piece = square -> piece;
                // if user clicked a move option square
                if (square -> isHighlighted)
                {
                    // move the piece the user is moving
                    std::vector<Piece*> piecesRemoved;
                    movePiece(pieceClicked, square, piecesRemoved);
                    // remember the last piece moved for en passant rights
                    lastMoved = pieceClicked;

                    makeBestMove();
                }
                resetHighlights();
                if (piece != nullptr && piece -> isWhite == ENGINE_IS_BLACK)
                {
                    // remember piece the player wants to move
                    pieceClicked = piece;
                    // highlight legal moves
                    std::vector<Square*> moves = getMoves(piece);
                    for (Square* &move : moves)
                    {
                        move -> isHighlighted = true;
                    }
                }
            }
        }
    }
}

void Board::makeBestMove()
{
    // for now, we will select a random move to serve as a fake engine
    std::vector<Square*> moves;
    Piece *pieceToMove;

    int tries = 0;
    while (moves.empty())
    {
        // choose a random piece of the color we want to move
        if (ENGINE_IS_BLACK)
        {
            // no legal moves possible because there are no pieces. this will never happen in real chess.
            if (blackPieces.empty())
            {
                // don't move
                return;
            }
            pieceToMove = blackPieces[rand() % blackPieces.size()];
        }
        else
        {
            // no legal moves possible becaue there are no pieces. this will never happen in real chess.
            if (whitePieces.empty())
            {
                // don't move
                return;
            }
            pieceToMove = whitePieces[rand() % whitePieces.size()];
        }
        moves = getMoves(pieceToMove);

        // this means we looked through all our pieces and none of them have any moves
        if (tries++ > 64)
        {
            // so just don't move
            return;
        }
    }

    Square *move = moves[rand() % moves.size()];
    std::vector<Piece*> piecesRemoved;
    movePiece(pieceToMove, move, piecesRemoved);

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

    // if we are moving a pawn
    if (type == PAWN)
    {
        // figure out which way this pawn is going
        int direction = isWhite ? 1 : -1;
        if (ENGINE_IS_BLACK)
        {
            direction *= -1;
        }

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
            // check for left en passant capture
            Piece *passantCapture = board[row][col - 1] -> piece;
            if (passantCapture == lastMoved && passantCapture -> timesMoved == 1)
            {
                if (passantCapture -> type == PAWN && (row == 3 || row == 4))
                {
                    moves.push_back(board[row + direction][col - 1]);
                }
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
            // check for right en passant capture
            Piece *passantCapture = board[row][col + 1] -> piece;
            if (passantCapture == lastMoved && passantCapture -> timesMoved == 1)
            {
                if (passantCapture -> type == PAWN && (row == 3 || row == 4))
                {
                    moves.push_back(board[row + direction][col + 1]);
                }
            }

        }

    }
    // if we are moving a knight
    else if (type == KNIGHT)
    {
        // offsets for how a knight moves
        int offsets[8][2] = {{-2, 1}, {-2, -1}, {2, -1}, {2, 1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

        // go through the offsets
        for (auto &offset : offsets)
        {
            // figure out what square we landed on
            int r = row + offset[0];
            int c = col + offset[1];
            if (isOnBoard(r, c))
            {
                Square *move = board[r][c];
                // if our landing square is empty or is a capture square
                if (move -> piece == nullptr || move -> piece -> isWhite != isWhite)
                {
                    // that square is a legal knight move
                    moves.push_back(board[r][c]);
                }
            }
        }
    }
    // if we are moving a king
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
                    // make sure we don't go off the board
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
        // check if we can castle
        if (moving -> timesMoved == 0)
        {
            Piece *leftRook = board[row][0] -> piece;
            Piece *rightRook = board[row][7] -> piece;
            // check if we can castle left
            if (leftRook != nullptr && leftRook -> timesMoved == 0)
            {
                // make sure no pieces are in between rook and king
                for (int x = 1; x < col; x++)
                {
                    if (board[row][x] -> piece != nullptr)
                    {
                        break;
                    }
                    if (x == col - 1)
                    {
                        moves.push_back(board[row][col - 2]);
                    }
                }
            }
            // check if we can castle right
            if (rightRook != nullptr && rightRook -> timesMoved == 0)
            {
                // make sure no pieces are in between rook and king
                for (int x = col + 1; x < rightRook -> col; x++)
                {
                    if (board[row][x] -> piece != nullptr)
                    {
                        break;
                    }
                    if (x == rightRook -> col - 1)
                    {
                        moves.push_back(board[row][col + 2]);
                    }
                }
            }
        }
    }
    // if we are moving bishop or queen
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
                // if the destination square was an empty square
                if (move -> piece == nullptr)
                {
                    // add it as a move
                    moves.push_back(move);
                }
                // if the destination square was a capture square
                else if (move -> piece -> isWhite != isWhite)
                {
                    // add it as a move
                    moves.push_back(move);
                    // don't jump over captures
                    break;
                }
                else
                {
                    // don't jump over pieces
                    break;
                }
            }
        }
    }
    // if we are moving rook or queen
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
                // if the destination square is an empty square
                if (move -> piece == nullptr)
                {
                    moves.push_back(move);
                }
                // if the destination square is a capture square
                else if (move -> piece -> isWhite != isWhite)
                {
                    // don't jump over captures
                    moves.push_back(move);
                    break;
                }
                else
                {
                    // don't jump over pieces
                    break;
                }
            }
        }
    }

    return moves;

}
