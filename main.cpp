#include "Board.h"

SDL_Renderer *renderer;
SDL_Window *window;
Board board;

void init()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_SIZE, WINDOW_SIZE, 0, &window, &renderer);
}

void run()
{
    SDL_Event event;
    Piece *pieceClicked;

    while (true)
    {
        SDL_RenderClear(renderer);
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                pieceClicked = board.onClicked(event.button.x, event.button.y);
            }
            if (event.type == SDL_QUIT)
            {
                return;
            }
        }
        board.render(renderer);
        SDL_RenderPresent(renderer);
    }
}

void stop()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

int main() {
    init();
    run();
    stop();
}
