//
// Created by Joe Chrisman on 2/9/22.
//

#include "Square.h"

Square::Square(int row, int col)
{
    this -> row = row;
    this -> col = col;
    piece = nullptr;
    bounds = new SDL_Rect{col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
    isHighlighted = false;
}

void Square::render(SDL_Renderer *renderer)
{
    // if the square should be highlighted
    if (isHighlighted)
    {
        SDL_SetRenderDrawColor(renderer, HIGHLIGHT_R, HIGHLIGHT_G, HIGHLIGHT_B, 255);
    }
    // if square should be a light square
    else if (row % 2 == col % 2)
    {
        SDL_SetRenderDrawColor(renderer, LIGHT_R, LIGHT_G, LIGHT_B, 255);
    }
    // if square should be a dark square
    else
    {
        SDL_SetRenderDrawColor(renderer, DARK_R, DARK_G, DARK_B, 255);
    }
    SDL_RenderFillRect(renderer, bounds);

    // if there is a piece on this square render the piece
    if (piece != nullptr)
    {
        piece -> render(renderer, bounds);
    }
}
