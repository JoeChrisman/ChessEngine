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
                addPiece(addedPiece);
            }
        }
        // add the rank to the board
        board[row] = rank;
    }

    // make engine move if engine is white
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

void Board::addPiece(Piece *toAdd)
{
    if (toAdd -> isWhite)
    {
        whitePieces.push_back(toAdd);
        if (toAdd -> type == KING)
        {
            whiteKing = toAdd;
        }
    }
    else
    {
        blackPieces.push_back(toAdd);
        if (toAdd -> type == KING)
        {
            blackKing = toAdd;
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


// move a piece to a target square
// this function returns a vector that looks like this:
// piecesRemoved[0] = piece captured
// piecesRemoved[1] = pawn captured by en passant
// piecesRemoved[2] = pawn removed when promoting
std::vector<Piece*> Board::movePiece(int rowFrom, int colFrom, int rowTo, int colTo)
{
    std::vector<Piece*> piecesRemoved(3);
    Piece *moving = board[rowFrom][colFrom] -> piece;
    Square *destination = board[rowTo][colTo];
    Piece *captured = destination -> piece;
    int type = moving -> type;
    bool isWhite = moving -> isWhite;

    // remove piece from its square
    board[rowFrom][colFrom] -> piece = nullptr;

    // if we are capturing a piece
    if (captured != nullptr)
    {
        // remember the piece we captured
        piecesRemoved[0] = captured;
        removePiece(captured);
    }

    // if we are capturing en passant
    if (type == PAWN && captured == nullptr && colFrom != colTo)
    {
        // remember captured pawn
        Piece *enPassantCapture = board[rowFrom][colTo] -> piece;
        piecesRemoved[1] = enPassantCapture;
        // remove captured pawn
        removePiece(enPassantCapture);
        board[rowFrom][colTo] -> piece = nullptr;
    }

    // if we want to promote a pawn
    if (type == PAWN && (rowTo == 0 || rowTo == 7))
    {
        // remember pawn we are promoting
        piecesRemoved[2] = moving;
        // remove pawn we are promoting
        removePiece(moving);
        // create a queen and put it on the square we are promoting
        Piece *queen = new Piece(isWhite, QUEEN, rowTo, colTo);
        lastMoved = queen;
        addPiece(queen);
        destination -> piece = queen;
    }
    else
    {
        // move the piece
        moving -> row = rowTo;
        moving -> col = colTo;
        moving -> timesMoved++;
        destination -> piece = moving;
        lastMoved = moving;

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
    }
    return piecesRemoved;
}

void Board::movePieceBack(int rowFrom, int colFrom, int rowTo, int colTo, std::vector<Piece*> &piecesRemoved)
{
    Piece *moving = board[rowFrom][colFrom] -> piece;
    Piece *captured = piecesRemoved[0];

    // if we captured en passant
    if (piecesRemoved[1] != nullptr)
    {
        // restore pawn captured by en passant
        addPiece(piecesRemoved[1]);
        board[rowTo][colFrom] -> piece = piecesRemoved[1];
    }
    // if we promoted a pawn
    if (piecesRemoved[2] != nullptr)
    {
        // restore pawn we removed while promoting
        addPiece(piecesRemoved[2]);
        board[rowTo][colTo] -> piece = piecesRemoved[2];
        // remove the queen we promoted
        removePiece(board[rowFrom][colFrom] -> piece);
    }
    else
    // move the piece back to where it came from
    {
        moving -> row = rowTo;
        moving -> col = colTo;
        moving -> timesMoved--;
        board[rowTo][colTo] -> piece = moving;

        // if we want to undo castling
        if (moving -> type == KING && abs(colFrom - colTo) > 1)
        {
            // if we want to un-castle from the left
            if (colFrom < 4)
            {
                // move rook from right of king to left of board
                Piece *leftRook = board[rowFrom][colFrom + 1] -> piece;
                leftRook -> col = 0;
                leftRook -> timesMoved--;
                board[rowFrom][colFrom + 1] -> piece = nullptr;
                board[rowTo][0] -> piece = leftRook;
            }
                // if we want to un-castle from the right
            else
            {
                // move rook from left of king to right of board
                Piece *rightRook = board[rowFrom][colFrom - 1] -> piece;
                rightRook -> col = 7;
                rightRook -> timesMoved--;
                board[rowFrom][colFrom - 1] -> piece = nullptr;
                board[rowTo][7] -> piece = rightRook;
            }
        }
    }

    // restore captured piece
    board[rowFrom][colFrom] -> piece = captured;
    if (captured != nullptr)
    {
        addPiece(captured);
    }
}

void Board::onClicked(int x, int y, SDL_Renderer *renderer)
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
                    std::vector<Piece*> piecesRemoved(3);
                    movePiece(pieceClicked -> row, pieceClicked -> col, square -> row, square -> col);
                    int legalMoves = 0;
                    resetHighlights();

                    // update graphics with the user's move
                    SDL_RenderClear(renderer);
                    render(renderer);
                    SDL_RenderPresent(renderer);

                    // if the engine had no moves
                    if (!makeBestMove())
                    {
                        // if the engine is in checkmate
                        if (isInCheck(!ENGINE_IS_BLACK))
                        {
                            std::cout << "   |   VICTORY   |   \n";
                        }
                        // if the engine is in stalemate
                        else
                        {
                            std::cout << "   |   STALEMATE   |   \n";
                        }
                        isRunning = false;
                        return;
                    }
                    // if the engine played a move
                    else
                    {
                        // update graphics
                        SDL_RenderClear(renderer);
                        render(renderer);
                        SDL_RenderPresent(renderer);
                        // see how many legal moves the player has
                        int legalMoves = 0;
                        std::vector<Piece*> playerPieces = ENGINE_IS_BLACK ? whitePieces : blackPieces;
                        for (Piece *playerPiece : playerPieces)
                        {
                            legalMoves += getLegalMoves(playerPiece -> row, playerPiece -> col).size();
                        }
                        // if the player has no legal moves
                        if (legalMoves == 0)
                        {
                            // if the player is in checkmate
                            if (isInCheck(ENGINE_IS_BLACK))
                            {
                                std::cout << "   |   DEFEAT   |   \n";
                            }
                            else
                            {
                                std::cout << "   |   STALEMATE   |   \n";
                            }
                            isRunning = false;
                            return;
                        }
                    }
                }
                // if user clicked a piece to move it somewhere
                else if (!square -> isHighlighted && piece != nullptr && piece -> isWhite == ENGINE_IS_BLACK)
                {
                    resetHighlights();
                    // remember piece the player wants to move
                    pieceClicked = piece;
                    // highlight legal moves
                    std::vector<Square*> moves = getLegalMoves(piece -> row, piece -> col);
                    for (Square* &move : moves)
                    {
                        move -> isHighlighted = true;
                    }
                    // update graphics
                    SDL_RenderClear(renderer);
                    render(renderer);
                    SDL_RenderPresent(renderer);
                }
                // reset highlights when user clicks elsewhere
                else
                {
                    resetHighlights();
                    // update graphics
                    SDL_RenderClear(renderer);
                    render(renderer);
                    SDL_RenderPresent(renderer);
                }
                return;
            }
        }
    }
}

// return a high value when the engine is winning, and a low value when the engine is losing
int Board::evaluate()
{
    std::vector<Piece*> &enginePieces = ENGINE_IS_BLACK ? blackPieces : whitePieces;
    std::vector<Piece*> &playerPieces = ENGINE_IS_BLACK ? whitePieces : blackPieces;
    Piece *engineKing = ENGINE_IS_BLACK ? blackKing : whiteKing;
    Piece *playerKing = ENGINE_IS_BLACK ? whiteKing : blackKing;
    int originalKingCol = ENGINE_IS_BLACK ? 4 : 3;

    // the order here corresponds to the ordering of PIECE_TYPE in Constants.h
    int pieceScores[] = {100, 1500, 1900, 2500, 5000, 0};
    int score = 0;

    // go through the engine's pieces
    for (Piece *enginePiece : enginePieces)
    {
        PIECE_TYPE type = enginePiece -> type;
        // the engine gained some material advantage
        score += pieceScores[type];
        if (type == KNIGHT || type == BISHOP || type == PAWN)
        {
            // an engine's piece in the center is a good piece for the engine
            if (enginePiece -> row > 1 && enginePiece -> col > 1 &&
                enginePiece -> row < 6 && enginePiece -> col < 6)
            {
                // the engine gained some positional advantage
                score += pieceScores[PAWN] / (type == PAWN ? 5 : 3);
            }

            if (type == PAWN)
            {
                if (enginePiece -> row > 2 && enginePiece -> col > 2 &&
                    enginePiece -> row < 5 && enginePiece -> col < 5)
                {
                    // the engine having a pawn in the center four squares is good for the engine
                    score += pieceScores[PAWN] / 2;
                    if (enginePiece -> row == (ENGINE_IS_BLACK ? 3 : 2))
                    {
                        // the engine having a pawn in the center pushed three squares is good for the engine
                        score += pieceScores[PAWN];
                    }
                }
            }
        }
        // an engine piece that hasn't moved is bad for the engine
        if (type == KNIGHT || type == BISHOP)
        {
            if (enginePiece -> timesMoved == 0)
            {
                // the engine lost some positional advantage
                score -= pieceScores[PAWN] / 10;
            }
        }
        // the engine's castled king is bad for the player
        if (type == KING && enginePiece -> timesMoved == 1 && abs(enginePiece -> col - originalKingCol) > 1)
        {
            // the engine gained some king safety advantage
            score += pieceScores[PAWN];
        }
    }

    for (Piece *playerPiece : playerPieces)
    {
        PIECE_TYPE type = playerPiece -> type;
        // the engine lost some material advantage
        score -= pieceScores[type];

        // a player's piece in the center is a bad piece for the engine
        if (type == KNIGHT || type == BISHOP || type == PAWN)
        {
            if (playerPiece -> row > 1 && playerPiece -> col > 1 &&
                playerPiece -> row < 6 && playerPiece -> col < 6)
            {
                // the engine lost some positional advantage
                score -= pieceScores[PAWN] / (type == PAWN ? 5 : 3);
                if (type == PAWN)
                {
                    if (playerPiece -> row > 2 && playerPiece -> col > 2 &&
                        playerPiece -> row < 5 && playerPiece -> col < 5)
                    {
                        // the player having a pawn in the center four squares is bad for the engine
                        score -= pieceScores[PAWN] / 2;
                        if (playerPiece -> row == (ENGINE_IS_BLACK ? 2 : 3))
                        {
                            // the player having a pawn in the center pushed three squares is bad for the engine
                            score -= pieceScores[PAWN];
                        }
                    }
                }
            }
        }
        // a player's piece that hasn't moved is bad for the player
        if (type == KNIGHT || type == BISHOP)
        {
            if (playerPiece -> timesMoved == 0)
            {
                // the engine gained some positional advantage
                score += pieceScores[PAWN] / 10;
            }
        }
        // the player's castled king is bad for the engine
        if (type == KING && playerPiece -> timesMoved == 1 && abs(playerPiece -> col - originalKingCol) > 1)
        {
            // the engine lost some attacking advantage
            score -= pieceScores[PAWN];
        }
    }

    return score;
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

bool Board::isSquareAttacked(Square *square, bool attackerIsWhite)
{

    std::vector<Piece*> attackers = attackerIsWhite ? whitePieces : blackPieces;
    // go through attacking pieces
    for (Piece *attacker : attackers)
    {
        std::vector<Square*> moves = getMoves(attacker);
        // look at all the attacks
        for (Square *move : moves)
        {
            if (move == square)
            {
                return true;
            }
        }
    }
    return false;
}

bool Board::isInCheck(bool kingIsWhite)
{
    Piece *king = kingIsWhite ? whiteKing : blackKing;
    return isSquareAttacked(board[king -> row][king -> col], !kingIsWhite);
}

// get the squares a piece can move to accounting for check rules
std::vector<Square*> Board::getLegalMoves(int row, int col)
{
    Piece *moving = board[row][col] -> piece;
    std::vector<Square*> legalMoves;
    std::vector<Square*> moves = getMoves(moving);

    bool canCastleLeft = true;
    bool canCastleRight = true;

    for (Square* &move : moves)
    {
        // if we are castling
        if (moving -> type == KING && abs(move -> col - col) > 1)
        {
            // if we are castling right don't castle out of check or through check
            if (move -> col > 4)
            {
                for (int x = moving -> col; x <= moving -> col + 1; x++)
                {
                    if (isSquareAttacked(board[moving -> row][x], !moving -> isWhite))
                    {
                        canCastleRight = false;
                        break;
                    }
                }
                if (!canCastleRight)
                {
                    // ignore this move and go to the next one
                    continue;
                }
            }
            // if we are castling left don't castle out of check or through check
            else
            {
                for (int x = moving -> col - 1; x <= moving -> col; x++)
                {
                    if (isSquareAttacked(board[moving -> row][x], !moving -> isWhite))
                    {
                        canCastleLeft = false;
                        break;
                    }
                }
                if (!canCastleLeft)
                {
                    // ignore this move and go to the next one
                    continue;
                }
            }
        }
        // move the piece
        Piece *movedBefore = lastMoved;
        std::vector<Piece*> piecesRemoved = movePiece(row, col, move -> row, move -> col);

        // if we moved a piece and put ourselves in check, that move was illegal
        if (!isInCheck(moving -> isWhite))
        {
            legalMoves.push_back(move);
        }

        // move the piece back
        movePieceBack(move -> row, move -> col, row, col, piecesRemoved);
        lastMoved = movedBefore;
    }
    return legalMoves;
}

int Board::minimax(int depth, bool isMaximizing)
{
    // if we are done evaluating for this line return the evaluation of the position
    if (depth >= MAX_DEPTH)
    {
        return evaluate();
    }
    // if we are evaluating a move for the engine
    if (isMaximizing)
    {
        // we want to maximize the returning evaluations, so start with the smallest possible number
        int bestEval = -MAX_EVAL;
        bool foundMove = false;
        std::vector<Piece*> enginePieces = ENGINE_IS_BLACK ? blackPieces : whitePieces;
        // go through each of the engine's pieces
        for (Piece *enginePiece : enginePieces)
        {
            int row = enginePiece -> row;
            int col = enginePiece -> col;
            // go through each of the piece's moves
            std::vector<Square*> moves = getLegalMoves(row, col);
            for (Square *move : moves)
            {
                // move the piece
                Piece *movedBefore = lastMoved;
                std::vector<Piece*> piecesRemoved = movePiece(row, col, move -> row, move -> col);

                // recursively generate optimal evaluations for future moves
                int evaluation = minimax(depth + 1, !isMaximizing);
                if (evaluation >= bestEval)
                {
                    bestEval = evaluation;
                    foundMove = true;
                }

                // move the piece back
                movePieceBack(move -> row, move -> col, row, col, piecesRemoved);
                lastMoved = movedBefore;
            }
        }
        if (!foundMove)
        {
            // checkmate
            if (isInCheck(!ENGINE_IS_BLACK))
            {
                // a faster checkmate should have a lower evaluation
                return -MAX_EVAL + depth;
            }
            // stalemate
            return 0;
        }
        // return the highest evaluation we could find
        return bestEval;
    }
    // if we are evaluating a move for the player
    else
    {
        // we want to minimize the returning evaluations, so start with the largest possible number
        int bestEval = MAX_EVAL;
        bool foundMove = false;
        std::vector<Piece*> playerPieces = ENGINE_IS_BLACK ? whitePieces : blackPieces;
        // go through each of the player's pieces
        for (Piece *playerPiece : playerPieces)
        {
            int row = playerPiece -> row;
            int col = playerPiece -> col;
            // go through each of the piece's moves
            std::vector<Square*> moves = getLegalMoves(row, col);
            for (Square *move : moves)
            {
                // move the piece
                Piece *movedBefore = lastMoved;
                std::vector<Piece*> piecesRemoved = movePiece(row, col, move -> row, move -> col);

                // recursively generate optimal evaluations for future moves
                int evaluation = minimax(depth + 1, !isMaximizing);
                if (evaluation <= bestEval)
                {
                    bestEval = evaluation;
                    foundMove = true;
                }

                // move the piece back
                movePieceBack(move -> row, move -> col, row, col, piecesRemoved);
                lastMoved = movedBefore;
            }
        }
        if (!foundMove)
        {
            // checkmate
            if (isInCheck(ENGINE_IS_BLACK))
            {
                // a faster checkmate should have a higher evaluation
                return MAX_EVAL - depth;
            }
            // stalemate
            return 0;
        }
        // return the lowest evaluation we could find
        return bestEval;
    }
}

// make the engine make what it thinks is the best move
// returns true if the engine had a legal move
bool Board::makeBestMove()
{
    // we want a piece to move to a square
    Piece *chosenPiece = nullptr;
    Square *chosenSquare = nullptr;

    // we want to choose the move with the highest minimax evaluation
    int bestEval = -MAX_EVAL;
    // go through each of the engine's pieces
    std::vector<Piece*> enginePieces = ENGINE_IS_BLACK ? blackPieces : whitePieces;
    for (Piece *enginePiece : enginePieces)
    {
        int row = enginePiece -> row;
        int col = enginePiece -> col;
        // go through each of the piece's moves
        std::vector<Square*> moves = getLegalMoves(row, col);
        for (Square *move : moves)
        {
            // move the piece
            Piece *movedBefore = lastMoved;
            std::vector<Piece*> piecesRemoved = movePiece(row, col, move -> row, move -> col);
            // evaluate the move recursively starting by minimizing evaluations (because the minimizing player is next)
            int evaluation = minimax(0, false);
            if (evaluation >= bestEval)
            {
                bestEval = evaluation;
                chosenPiece = enginePiece;
                chosenSquare = move;
            }
            // move the piece back
            movePieceBack(move -> row, move -> col, row, col, piecesRemoved);
            lastMoved = movedBefore;
            /*
            std::cout << "piece address: " << enginePiece << "   |   ";
            std::cout << "from: (" << row << ", " << col << ")   |   ";
            std::cout << "to: (" << move -> row << ", " << move -> col << ")   |   ";
            std::cout << "legalMoves: " << moves.size() << "   |   ";
            std::cout << "evaluation: " << evaluation << std::endl;*/
        }
    }
    // if there were no legal moves
    if (chosenPiece == nullptr || chosenSquare == nullptr)
    {
        isRunning = false;
        return false;
    }
    else
    {
        // if we found a move make the engine's top move
        movePiece(chosenPiece -> row, chosenPiece -> col, chosenSquare -> row, chosenSquare -> col);
        /*
        std::cout << "white pieces: " << whitePieces.size() << std::endl;
        std::cout << "black pieces: " << blackPieces.size() << std::endl;
        std::cout << "current evaluation: " << bestEval << std::endl;*/
    }
    return true;

}















