#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <unistd.h>

SDL_Window *window;

bool process_key_stroke(SDL_Keysym symbol) {
    bool should_quit = false;

    switch (symbol.sym) {
    case SDLK_q:
        should_quit = true;
        break;

    default:
        printf("I found this [%d]\n", symbol.sym);
        break;
    }

    return should_quit;
}

bool handle_event(SDL_Event *event, SDL_Renderer *renderer) {
    bool should_quit = false;
    int r, g, b;

    switch (event->type) {
    case SDL_QUIT:
        should_quit = true;
        break;

    case SDL_KEYDOWN:
        r = event->key.keysym.sym % 255;
        g = (event->key.keysym.sym * 2) % 255;
        b = (event->key.keysym.sym * event->key.keysym.sym) % 255;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        should_quit = process_key_stroke(event->key.keysym);
        break;
    }

    return (should_quit);
}

int run_display_loop(void) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        // TODO(ryan): Uh oh...
    }

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

#ifdef DEBUG
    window_x_pos = 5120 - 740;
    window_y_pos = 100;
#endif

    window = SDL_CreateWindow("vcr display", window_x_pos, window_y_pos, 640, 480, SDL_WINDOW_BORDERLESS);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    while (true) {
        SDL_Event Event;
        SDL_WaitEvent(&Event);

        if (handle_event(&Event, renderer)) {
            break;
        }
    }

    SDL_Quit();
    return (0);
}
