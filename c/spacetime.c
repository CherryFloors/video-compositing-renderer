#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

const int HEIGHT = 1080;
const int WIDTH = HEIGHT;
const float PI = 3.14159265358979323846;

typedef struct Point {
    float x;
    float y;
    float z;
} Point;

void set_color_white(SDL_Renderer *renderer) { SDL_SetRenderDrawColor(renderer, 0xaa, 0xaa, 0xaa, 0xff); }

void set_color_black(SDL_Renderer *renderer) { SDL_SetRenderDrawColor(renderer, 0x10, 0x10, 0x10, 0xff); }

void set_color_green(SDL_Renderer *renderer) { SDL_SetRenderDrawColor(renderer, 0x50, 0xFF, 0x50, 0xff); }

void render_point(SDL_Renderer *renderer, Point point) {
    float s = 20.0;
    set_color_green(renderer);

    SDL_FRect frect = {point.x - s / 2, point.y - s / 2, s, s};
    SDL_RenderFillRectF(renderer, &frect);
}

void render_line(SDL_Renderer *renderer, Point p1, Point p2) {
    set_color_green(renderer);
    printf("RENDER (%f, %f) -> (%f, %f)\n", p1.x, p1.y, p2.x, p2.y);

    SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
    // SDL_FRect frect = {point.x - s / 2, point.y - s / 2, s, s};
    // SDL_RenderFillRectF(renderer, &frect);
}

Point screen(Point p) { return (Point){(p.x + 1) / 2 * WIDTH, (1 - (p.y + 1) / 2) * HEIGHT, p.z}; }

Point translate_z(Point p, float dz) { return (Point){p.x, p.y, p.z + dz}; }

Point project(Point p) { return (Point){p.x / p.z, p.y / p.z, p.z}; }

Point rotate_xz(Point p, float angle) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    return (Point){
        p.x * cos_a - p.z * sin_a,
        p.y,
        p.x * sin_a + p.z * cos_a,
    };
}

void render_clear(SDL_Renderer *renderer) {

    // set_color_white(renderer);
    set_color_black(renderer);
    SDL_RenderClear(renderer);
}

int spacetime(void) {

    printf("This is spacetime\n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to init SDL\n");
        return 1;
    }

    if (TTF_Init()) {
        printf("Failed to init TTF\n");
        return 1;
    }

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

    SDL_Window *window =
        SDL_CreateWindow("vcr display", window_x_pos, window_y_pos, WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    const int TARGET_FPS = 60;
    const int FRAME_DELAY = 1000 / TARGET_FPS;
    float dt = 1 / (float)TARGET_FPS;
    float dz = 0;
    float angle = 0;

    Point vs[] = {
        (Point){0.5, 0.5, 0.5},  (Point){-0.5, 0.5, 0.5},  (Point){-0.5, -0.5, 0.5},  (Point){0.5, -0.5, 0.5},

        (Point){0.5, 0.5, -0.5}, (Point){-0.5, 0.5, -0.5}, (Point){-0.5, -0.5, -0.5}, (Point){0.5, -0.5, -0.5},
    };
    int vs_len = sizeof(vs) / sizeof(vs[0]);

    int fs[][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };
    int fs_len = sizeof(fs) / sizeof(fs[0]);

    // int tr[][3] = {
    //     {0, 1, 2},
    //     {2, 3, 4},
    //     {4, 5},
    //     {6, 7},
    //     {0, 4},
    //     {1, 5},
    //     {2, 6},
    //     {3, 7},
    // };
    // int tr_len = sizeof(tr) / sizeof(tr[0]);

    bool running = true;
    while (running) {

        dz += 1.0 * dt;
        angle += 1.0 * PI * dt;

        int frame_start = SDL_GetTicks();
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {

            switch (Event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                if (Event.key.keysym.sym == SDLK_q) {
                    running = false;
                }
                break;
            }
        }

        render_clear(renderer);
        // for (int i=0; i<vs_len; i++) {
        //     render_point(renderer, screen(project(translate_z(vs[i], dz))));
        // }
        for (int i = 0; i < fs_len; i++) {
            Point a = screen(project(translate_z(rotate_xz(vs[fs[i][0]], angle), dz)));
            Point b = screen(project(translate_z(rotate_xz(vs[fs[i][1]], angle), dz)));
            render_line(renderer, a, b);
        }
        SDL_RenderPresent(renderer);

        int frame_time = SDL_GetTicks() - frame_start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    window = NULL;
    renderer = NULL;

    SDL_Quit();

    return 0;
}
