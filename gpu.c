#include "./common.h"

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

    GLuint vert_shader = compile_shader_file("./gpu.vert", GL_VERTEX_SHADER);
    GLuint frag_shader = compile_shader_file("./gpu.frag", GL_FRAGMENT_SHADER);
    GLuint program = link_program(vert_shader, frag_shader);
    glUseProgram(program);

    GLint u_p1 = glGetUniformLocation(program, "p1");
    GLint u_p2 = glGetUniformLocation(program, "p2");
    GLint u_p3 = glGetUniformLocation(program, "p3");
    GLint u_marker_radius = glGetUniformLocation(program, "marker_radius");
    GLint u_bezier_curve_threshold = glGetUniformLocation(program, "bezier_curve_threshold");

    Vec2 p1 = vec2(100.0f, 100.0f);
    Vec2 p2 = vec2(200.0f, 200.0f);
    Vec2 p3 = vec2(300.0f, 300.0f);
    Vec2 *p_selected = NULL;
    float bezier_curve_threshold = 10.0f;

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
                    // TODO: dragging does not work if the window size is not correct
                    if (vec2_length(vec2_sub(p1, mouse_pos)) < MARKER_SIZE) {
                        p_selected = &p1;
                    }

                    if (vec2_length(vec2_sub(p2, mouse_pos)) < MARKER_SIZE) {
                        p_selected = &p2;
                    }

                    if (vec2_length(vec2_sub(p3, mouse_pos)) < MARKER_SIZE) {
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

            case SDL_MOUSEWHEEL: {
#define BEZIER_CURVE_THRESHOLD_STEP 5.0f
                if (event.wheel.y > 0) {
                    bezier_curve_threshold = bezier_curve_threshold + BEZIER_CURVE_THRESHOLD_STEP;
                } else if (event.wheel.y < 0) {
                    bezier_curve_threshold = fmaxf(bezier_curve_threshold - BEZIER_CURVE_THRESHOLD_STEP, 0.0f);
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
        glUniform1f(u_marker_radius, MARKER_SIZE);
        glUniform1f(u_bezier_curve_threshold, bezier_curve_threshold);
        glDrawArrays(GL_QUADS, 0, 4);

        SDL_GL_SwapWindow(window);
    }


    return 0;
}
