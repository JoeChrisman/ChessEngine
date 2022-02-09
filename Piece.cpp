//
// Created by Joe Chrisman on 2/9/22.
//

#include "Piece.h"

Piece::Piece(bool isWhite, PIECE_TYPE type, int row, int col)
{
    this -> row = row;
    this -> col = col;
    this -> type = type;
    this -> isWhite = isWhite;
    timesMoved = 0;
}

void Piece::render(SDL_Renderer *renderer, SDL_Rect *bounds)
{
    // load .bmp format image
    SDL_Surface *surface = SDL_LoadBMP(getImagePath().c_str());
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_RenderCopy(renderer, texture, nullptr, bounds);
}

// get directory path to the image for this piece
// example: a white pawn would load a path like "images/wp.bmp"
std::string Piece::getImagePath()
{
    std::string root = "images/";
    std::string fileType = ".bmp";
    std::string colorCode = isWhite ? "w" : "b";
    std::string typeCode;
    switch(type)
    {
        case PAWN: typeCode = "p"; break;
        case KNIGHT: typeCode = "n"; break;
        case BISHOP: typeCode = "b"; break;
        case ROOK: typeCode = "r"; break;
        case QUEEN: typeCode = "q"; break;
        case KING: typeCode = "k"; break;
    }
    return root + colorCode + typeCode + fileType;
}
