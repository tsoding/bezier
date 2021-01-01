#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_FPS 60
#define DELTA_TIME_SEC (1.0f / SCREEN_FPS)
#define DELTA_TIME_MS ((Uint32) floorf(DELTA_TIME_SEC * 1000.0f))
#define MARKER_SIZE 15.0f
#define BACKGROUND_COLOR 0x353535FF
#define RED_COLOR 0xDA2C38FF
#define GREEN_COLOR 0x87C38FFF
#define BLUE_COLOR 0x748CABFF

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

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return vec2(a.x - b.x, a.y - b.y);
}

Vec2 vec2_scale(Vec2 a, float s)
{
    return vec2(a.x * s, a.y * s);
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return vec2(a.x + b.x, a.y + b.y);
}

float lerpf(float a, float b, float p)
{
    return a + (b - a) * p;
}

Vec2 lerpv2(Vec2 a, Vec2 b, float p)
{
    return vec2_add(a, vec2_scale(vec2_sub(b, a), p));
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

void render_marker(SDL_Renderer *renderer, Vec2 pos, uint32_t color)
{
    const Vec2 size = vec2(MARKER_SIZE, MARKER_SIZE);
    fill_rect(
        renderer,
        vec2_sub(pos, vec2_scale(size, 0.5f)),
        size,
        color);
}

void render_bezier_markers(SDL_Renderer *renderer,
                           Vec2 a, Vec2 b, Vec2 c, Vec2 d,
                           float s, uint32_t color)
{
    for (float p = 0.0f; p <= 1.0f; p += s) {
        Vec2 ab = lerpv2(a, b, p);
        Vec2 bc = lerpv2(b, c, p);
        Vec2 cd = lerpv2(c, d, p);
        Vec2 abc = lerpv2(ab, bc, p);
        Vec2 bcd = lerpv2(bc, cd, p);
        Vec2 abcd = lerpv2(abc, bcd, p);
        render_marker(renderer, abcd, color);
    }
}

#define PS_CAPACITY 256

Vec2 ps[PS_CAPACITY];
size_t ps_count = 0;

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
    float t = 0.0f;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = 1;
                break;

            case SDL_MOUSEBUTTONDOWN: {
                switch (event.button.button) {
                case SDL_BUTTON_LEFT: {
                    ps[ps_count++] =
                        vec2(event.button.x,
                             event.button.y);
                } break;
                }
            } break;
            }
        }

        check_sdl_code(
            SDL_SetRenderDrawColor(
                renderer,
                HEX_COLOR(BACKGROUND_COLOR)));

        check_sdl_code(
            SDL_RenderClear(renderer));

        for (size_t i = 0; i < ps_count; ++i) {
            render_marker(renderer, ps[i], RED_COLOR);
        }

        if (ps_count >= 4) {
            render_bezier_markers(
                renderer,
                ps[0], ps[1], ps[2], ps[3],
                0.05f,
                GREEN_COLOR);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(DELTA_TIME_MS);
        t += DELTA_TIME_SEC;
    }

    SDL_Quit();

    return 0;
}
