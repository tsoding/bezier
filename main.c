#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BACKGROUND_COLOR 0x353535FF
#define LINE_COLOR 0xDA2C38FF
#define RECT_COLOR 0x87C38FFF

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

typedef struct {
    float x;
    float y;
} Vec2;

Vec2 vec2(float x, float y)
{
    return (Vec2){x, y};
}

void render_line(SDL_Renderer *renderer,
                 Vec2 begin, Vec2 end,
                 uint32_t color)
{
    check_sdl_code(
        SDL_SetRenderDrawColor(renderer, HEX_COLOR(color)));

    check_sdl_code(
        SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x),
            (int) floorf(begin.y),
            (int) floorf(end.x),
            (int) floorf(end.y)));
}

void fill_rect(SDL_Renderer *renderer, Vec2 pos, Vec2 size, uint32_t color)
{
    check_sdl_code(
        SDL_SetRenderDrawColor(renderer, HEX_COLOR(color)));

    const SDL_Rect rect = {
        (int) floorf(pos.x),
        (int) floorf(pos.y),
        (int) floorf(size.x),
        (int) floorf(size.y),
    };

    check_sdl_code(SDL_RenderFillRect(renderer, &rect));
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

    check_sdl_code(
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT));

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

        render_line(
            renderer,
            vec2(0.0f, 0.0f),
            vec2(SCREEN_WIDTH, SCREEN_HEIGHT),
            LINE_COLOR);
        render_line(
            renderer,
            vec2(SCREEN_WIDTH, 0.0f),
            vec2(0.0f, SCREEN_HEIGHT),
            LINE_COLOR);

        fill_rect(renderer, vec2(0.0f, 0.0f), vec2(100.0f, 100.0f), RECT_COLOR);

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();

    return 0;
}
