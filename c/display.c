#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

SDL_Window *window;
SDL_Texture *text_texture;
TTF_Font *font;

typedef struct VcrColorPallete {
    SDL_Color c0;
    SDL_Color c1;
    SDL_Color c2;
    SDL_Color c3;
    SDL_Color c4;
    SDL_Color text_fg;
    SDL_Color text_bg;
} VcrColorPallete;

VcrColorPallete default_color_pallete(void) {
    VcrColorPallete vcr_color_pallete = {
        {0xFD, 0xC3, 0x31}, {0xFE, 0x57, 0x22}, {0xF0, 0x35, 0x3C}, {0xB4, 0x21, 0x3D},
        {0x67, 0x1C, 0x3B}, {0xFF, 0xFF, 0xFF}, {0x1A, 0x1A, 0x1A},
    };
    return vcr_color_pallete;
}

void standby_screen(SDL_Renderer *renderer) {
    VcrColorPallete colors = default_color_pallete();
    SDL_SetRenderDrawColor(renderer, colors.text_fg.r, colors.text_fg.g, colors.text_fg.b, 255);
    SDL_RenderClear(renderer);

    int padding = 125;
    int bar_height = (480 - padding * 2) / 5;

    SDL_Rect r0 = {0, padding + (bar_height * 0), 640, bar_height};
    SDL_Rect r1 = {0, padding + (bar_height * 1), 640, bar_height};
    SDL_Rect r2 = {0, padding + (bar_height * 2), 640, bar_height};
    SDL_Rect r3 = {0, padding + (bar_height * 3), 640, bar_height};
    SDL_Rect r4 = {0, padding + (bar_height * 4), 640, bar_height};

    SDL_SetRenderDrawColor(renderer, colors.c0.r, colors.c0.g, colors.c0.b, 255);
    SDL_RenderFillRect(renderer, &r0);

    SDL_SetRenderDrawColor(renderer, colors.c1.r, colors.c1.g, colors.c1.b, 255);
    SDL_RenderFillRect(renderer, &r1);

    SDL_SetRenderDrawColor(renderer, colors.c2.r, colors.c2.g, colors.c2.b, 255);
    SDL_RenderFillRect(renderer, &r2);

    SDL_SetRenderDrawColor(renderer, colors.c3.r, colors.c3.g, colors.c3.b, 255);
    SDL_RenderFillRect(renderer, &r3);

    SDL_SetRenderDrawColor(renderer, colors.c4.r, colors.c4.g, colors.c4.b, 255);
    SDL_RenderFillRect(renderer, &r4);

    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

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

        if (!font) {
            r = event->key.keysym.sym % 255;
            g = (event->key.keysym.sym * 2) % 255;
            b = (event->key.keysym.sym * event->key.keysym.sym) % 255;
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderClear(renderer);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            char text[50];
            SDL_snprintf(text, sizeof(text), "I found this [%d]", event->key.keysym.sym);
            SDL_Color foreground = {0x6E, 0xFB, 0x4C};
            SDL_Surface *text_surface = TTF_RenderText_Solid(font, text, foreground);

            float scale = 640.0 / text_surface->w;

            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_Rect dest = {0, 0, 640, (int)(text_surface->h * scale)};

            SDL_RenderCopy(renderer, text_texture, NULL, &dest);
            SDL_DestroyTexture(text_texture);
            SDL_FreeSurface(text_surface);
        }

        SDL_RenderPresent(renderer);
        should_quit = process_key_stroke(event->key.keysym);
        SDL_RenderClear(renderer);

        if (event->key.keysym.sym == SDLK_s) {
            standby_screen(renderer);
        }

        break;
    }

    return (should_quit);
}

int run_display_loop(void) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        // TODO(ryan): Uh oh...
        printf("Failed to init SDL\n");
        return 1;
    }

    if (TTF_Init()) {
        // TODO(ryan): Uh oh...
        printf("Failed to init TTF\n");
        return 1;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/UbuntuSansMono[wght].ttf", 36);
    if (!font) {
        // TODO(ryan): Uh oh...
        printf("Failed to open font\n");
    }

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

#ifdef DEBUG
    window_x_pos = 5120 - 740;
    window_y_pos = 100;
#endif

    window = SDL_CreateWindow("vcr display", window_x_pos, window_y_pos, 640, 480, SDL_WINDOW_BORDERLESS);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    standby_screen(renderer);
    while (true) {
        SDL_Event Event;
        SDL_WaitEvent(&Event);

        if (handle_event(&Event, renderer)) {
            break;
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyTexture(text_texture);
    text_texture = NULL;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    renderer = NULL;

    TTF_Quit();
    SDL_Quit();
    return (0);
}
