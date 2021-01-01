#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BACKGROUND_COLOR 0x000000FF

#define HEX_COLOR(hex)                      \
    ((hex) >> (3 * 8)) & 0xFF,              \
    ((hex) >> (2 * 8)) & 0xFF,              \
    ((hex) >> (1 * 8)) & 0xFF,              \
    ((hex) >> (0 * 8)) & 0xFF

int check_sdl_code(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        exit(1);
    }

    return code;
}

void *check_sdl_ptr(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        exit(1);
    }

    return ptr;
}

int main(void)
{
    check_sdl_code(
        SDL_Init(SDL_INIT_VIDEO));

    SDL_Window * const window =
        check_sdl_ptr(
            SDL_CreateWindow(
                "Bezier Curves",
                0, 0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                SDL_WINDOW_RESIZABLE));

    SDL_Renderer * const renderer =
        check_sdl_ptr(
            SDL_CreateRenderer(
                window, -1, SDL_RENDERER_ACCELERATED));

    int quit = 0;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            }
        }

        check_sdl_code(
            SDL_SetRenderDrawColor(
                renderer,
                HEX_COLOR(BACKGROUND_COLOR)));

        check_sdl_code(
            SDL_RenderClear(renderer));

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();

    return 0;
}
