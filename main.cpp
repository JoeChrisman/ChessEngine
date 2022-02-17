#include "Board.h"

SDL_Renderer *renderer;
SDL_Window *window;
Board board;

// create SDL window and renderer
void init()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_SIZE, WINDOW_SIZE, 0, &window, &renderer);
}

// initialize game render loop
void run()
{
    SDL_Event event;

    board.render(renderer);
    SDL_RenderPresent(renderer);
    while (true)
    {
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_MOUSEBUTTONDOWN && board.isRunning)
            {
                board.onClicked(event.button.x, event.button.y, renderer);
            }
            if (event.type == SDL_QUIT)
            {
                return;
            }
        }
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
