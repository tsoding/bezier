#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>

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

float vec2_length(Vec2 a)
{
    return sqrtf(a.x * a.x + a.y * a.y);
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

Vec2 beziern_sample(Vec2 *ps, Vec2 *xs, size_t n, float p)
{
    memcpy(xs, ps, sizeof(Vec2) * n);

    while (n > 1) {
        for (size_t i = 0; i < n - 1; ++i) {
            xs[i] = lerpv2(xs[i], xs[i + 1], p);
        }
        n -= 1;
    }

    return xs[0];
}

// TODO: explore how to render bezier curves on GPU using fragment shaders
void render_bezier_markers(SDL_Renderer *renderer,
                           Vec2 *ps, Vec2 *xs, size_t n,
                           float s, uint32_t color)
{
    for (float p = 0.0f; p <= 1.0f; p += s) {
        render_marker(renderer, beziern_sample(ps, xs, n, p), color);
    }
}

void render_bezier_curve(SDL_Renderer *renderer,
                         Vec2 *ps, Vec2 *xs, size_t n,
                         float s, uint32_t color)
{
    for (float p = 0.0f; p <= 1.0f; p += s) {
        Vec2 begin = beziern_sample(ps, xs, n, p);
        Vec2 end = beziern_sample(ps, xs, n, p + s);
        render_line(renderer, begin, end, color);
    }
}

#define PS_CAPACITY 256

Vec2 ps[PS_CAPACITY];
Vec2 xs[PS_CAPACITY];
size_t ps_count = 0;
int ps_selected = -1;

int ps_at(Vec2 pos)
{
    const Vec2 ps_size = vec2(MARKER_SIZE, MARKER_SIZE);
    for (size_t i = 0; i < ps_count; ++i) {
        const Vec2 ps_begin = vec2_sub(ps[i], vec2_scale(ps_size, 0.5f));
        const Vec2 ps_end = vec2_add(ps_begin, ps_size);
        if (ps_begin.x <= pos.x && pos.x <= ps_end.x &&
            ps_begin.y <= pos.y && pos.y <= ps_end.y) {
            return (int) i;
        }
    }
    return -1;
}

char *cstr_slurp_file(const char *file_path)
{
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
                file_path, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
                file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
                file_path, strerror(errno));
        exit(1);
    }

    char *buffer = malloc(m + 1);
    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for file: %s\n",
                strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
                file_path, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if (ferror(f)) {
        fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
                file_path, strerror(errno));
        exit(1);
    }

    buffer[n] = '\0';

    fclose(f);

    return buffer;
}

GLuint compile_shader_file(const char *file_path, GLenum shader_type)
{
    char *buffer = cstr_slurp_file(file_path);
    const GLchar * const source_cstr = buffer;

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source_cstr, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLchar log[1024];
        GLsizei log_size = 0;
        glGetShaderInfoLog(shader, sizeof(log), &log_size, log);
        fprintf(stderr, "Failed to compile %s:%.*s\n", file_path,
                log_size, log);
        exit(1);
    }

    free(buffer);

    return shader;
}

GLuint link_program(GLuint vert_shader, GLuint frag_shader)
{
    GLuint program = glCreateProgram();

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLsizei log_size = 0;
        GLchar log[1024];
        glGetProgramInfoLog(program, sizeof(log), &log_size, log);
        fprintf(stderr, "Failed to link the shader program: %.*s\n", log_size, log);
        exit(1);
    }

    return program;
}

#define MARKER_RADIUS 20.0f

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
                SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));

    /*SDL_GLContext context = */SDL_GL_CreateContext(window);

    GLuint vert_shader = compile_shader_file("./bezier.vert", GL_VERTEX_SHADER);
    GLuint frag_shader = compile_shader_file("./bezier.frag", GL_FRAGMENT_SHADER);
    GLuint program = link_program(vert_shader, frag_shader);
    glUseProgram(program);

    GLint u_p1 = glGetUniformLocation(program, "p1");
    GLint u_p2 = glGetUniformLocation(program, "p2");
    GLint u_p3 = glGetUniformLocation(program, "p3");
    GLint u_marker_radius = glGetUniformLocation(program, "marker_radius");

    Vec2 p1 = vec2(100.0f, 100.0f);
    Vec2 p2 = vec2(200.0f, 200.0f);
    Vec2 p3 = vec2(300.0f, 300.0f);
    Vec2 *p_selected = NULL;

    int quit = 0;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = 1;
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                const Vec2 mouse_pos = vec2(
                    event.button.x,
                    SCREEN_HEIGHT - event.button.y);
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (vec2_length(vec2_sub(p1, mouse_pos)) < MARKER_RADIUS) {
                        p_selected = &p1;
                    }

                    if (vec2_length(vec2_sub(p2, mouse_pos)) < MARKER_RADIUS) {
                        p_selected = &p2;
                    }

                    if (vec2_length(vec2_sub(p3, mouse_pos)) < MARKER_RADIUS) {
                        p_selected = &p3;
                    }
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    p_selected = NULL;
                }
            } break;

            case SDL_MOUSEMOTION: {
                const Vec2 mouse_pos = vec2(
                    event.button.x,
                    SCREEN_HEIGHT - event.button.y);
                if (p_selected) {
                    *p_selected = mouse_pos;
                }
            } break;
            }
        }

        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform2f(u_p1, p1.x, p1.y);
        glUniform2f(u_p2, p2.x, p2.y);
        glUniform2f(u_p3, p3.x, p3.y);
        glUniform1f(u_marker_radius, MARKER_RADIUS);
        glDrawArrays(GL_QUADS, 0, 4);

        SDL_GL_SwapWindow(window);
    }


    return 0;
}

int main2(void)
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
    int markers = 1;
    float t = 0.0f;
    float bezier_sample_step = 0.05f;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = 1;
                break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_F1: {
                    markers = !markers;
                } break;
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                switch (event.button.button) {
                case SDL_BUTTON_LEFT: {
                    const Vec2 mouse_pos = vec2(event.button.x, event.button.y);
                    ps_selected = ps_at(mouse_pos);

                    if (ps_selected < 0 && ps_count < PS_CAPACITY) {
                        ps[ps_count++] = mouse_pos;
                    }
                } break;
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    ps_selected = -1;
                }
            } break;

            case SDL_MOUSEMOTION: {
                Vec2 mouse_pos = vec2(event.motion.x, event.motion.y);
                if (ps_selected >= 0) {
                    ps[ps_selected] = mouse_pos;
                }
            } break;

            case SDL_MOUSEWHEEL: {
                if (event.wheel.y > 0) {
                    bezier_sample_step = fminf(bezier_sample_step + 0.001f, 0.999f);
                } else if (event.wheel.y < 0) {
                    bezier_sample_step = fmaxf(bezier_sample_step - 0.001f, 0.001f);
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

        if (ps_count >= 1) {
            if (markers) {
                render_bezier_markers(
                    renderer,
                    ps, xs, ps_count,
                    bezier_sample_step,
                    GREEN_COLOR);
            } else {
                render_bezier_curve(
                    renderer,
                    ps, xs, ps_count,
                    bezier_sample_step,
                    GREEN_COLOR);
            }
        }

        for (size_t i = 0; i < ps_count; ++i) {
            render_marker(renderer, ps[i], RED_COLOR);
            if (i < ps_count - 1) {
                render_line(renderer, ps[i], ps[i + 1], RED_COLOR);
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(DELTA_TIME_MS);
        t += DELTA_TIME_SEC;
    }

    SDL_Quit();

    return 0;
}
