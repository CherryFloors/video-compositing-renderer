#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

SDL_Window *window;
SDL_Texture *text_texture;
TTF_Font *font;
TTF_Font *smallfont;
TTF_Font *alphafont;

typedef struct VcrColorPalette {
    SDL_Color c0;
    SDL_Color c1;
    SDL_Color c2;
    SDL_Color c3;
    SDL_Color c4;
    SDL_Color text_fg;
    SDL_Color text_bg;
} VcrColorPalette;

VcrColorPalette default_color_palette(void) {
    VcrColorPalette vcr_color_palette = {
        {0xFD, 0xC3, 0x31}, {0xFE, 0x57, 0x22}, {0xF0, 0x35, 0x3C}, {0xB4, 0x21, 0x3D},
        {0x67, 0x1C, 0x3B}, {0xFF, 0xFF, 0xFF}, {0x1A, 0x1A, 0x1A},
    };
    return vcr_color_palette;
}

VcrColorPalette supercolor_palette(void) {
    VcrColorPalette vcr_color_palette = {
        {0x03, 0x8D, 0xAE}, {0x8E, 0xA4, 0x3B}, {0xE9, 0xBD, 0x01}, {0xF2, 0x47, 0x01},
        {0xF8, 0x01, 0x50}, {0xE8, 0xE1, 0xD6}, {0x18, 0x18, 0x18},
    };
    return vcr_color_palette;
}

void render_videoscreen(SDL_Renderer *renderer, SDL_Rect screen) {

    SDL_Rect shadow = screen;
    int shadow_size = 5;
    int shadow_alpha = 0xFF / 2;
    int alpha_decay = shadow_alpha / shadow_size;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i<shadow_size; i++) {
        shadow.x -= 1;
        shadow.y -= 1;
        shadow.w += 2;
        shadow.h += 2;
        shadow_alpha -= alpha_decay;
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, shadow_alpha);
        SDL_RenderDrawRect(renderer, &shadow);
    }

    SDL_SetRenderDrawColor(renderer, 0x1A, 0xFD, 0xD7, 0xFF);
    SDL_RenderFillRect(renderer, &screen);
}

void draw_standby_digital_display(SDL_Renderer *renderer, int x, int y, int w, int h) {

    int outw = 1;
    TTF_SetFontOutline(font, outw);
    TTF_SetFontOutline(alphafont, outw);

    SDL_Color display_bg                  = {0x00, 0x00, 0x00};
    SDL_Color solid_text_color            = {0x1A, 0xFD, 0xD7};
    SDL_Color active_text_outline_color   = {solid_text_color.r, solid_text_color.g, solid_text_color.b, 120};
    SDL_Color active_text_center_color    = {0xBD, 0xFF, 0xFD};
    SDL_Color inactive_text_outline_color = {0x3A, 0x3A, 0x3A};
    SDL_Color inactive_text_center_color  = {0x25, 0x25, 0x25};

    SDL_Surface *ampm = TTF_RenderText_Blended(alphafont, " PM", active_text_outline_color);
    SDL_Surface *inactive_surface = TTF_RenderText_Blended(font, "8888888:88", inactive_text_outline_color);
    SDL_Surface *ampm_surf = TTF_RenderText_Blended(alphafont, " PM", active_text_center_color);
    SDL_Surface *active_surface = TTF_RenderText_Blended(font, "VCR!!12:34", active_text_outline_color);
    TTF_SetFontOutline(alphafont, 0);
    TTF_SetFontOutline(font, 0);
    SDL_Surface *active_surface_no_outline = TTF_RenderText_Blended(font, "VCR!!12:34", active_text_center_color);
    SDL_Surface *inactive_surface_no_outline = TTF_RenderText_Blended(font, "8888888:88", inactive_text_center_color);

    int padding = w - inactive_surface->w;
    x = (640 - w + ampm->w + padding) / 2;
    SDL_Rect background = {x, y, w + ampm->w, inactive_surface->h + padding};
    SDL_Rect text_box = {x + padding / 2, y + padding / 2, inactive_surface->w, inactive_surface->h};
    SDL_Rect ampm_box = {text_box.x + text_box.w, (text_box.h - ampm->h) + text_box.y, ampm->w, ampm->h};
    SDL_Rect centered_textbox = {text_box.x + outw, text_box.y + outw, inactive_surface_no_outline->w, inactive_surface_no_outline->h};


    SDL_Texture *inactive_texture = SDL_CreateTextureFromSurface(renderer, inactive_surface);
    SDL_Texture *inactive_ampm = SDL_CreateTextureFromSurface(renderer, ampm);
    SDL_Texture *active_ampm = SDL_CreateTextureFromSurface(renderer, ampm_surf);
    SDL_Texture *active_texture = SDL_CreateTextureFromSurface(renderer, active_surface);
    SDL_Texture *inactive_texture_no_outline = SDL_CreateTextureFromSurface(renderer, inactive_surface_no_outline);
    SDL_Texture *active_texture_no_outline = SDL_CreateTextureFromSurface(renderer, active_surface_no_outline);

    // display bg
    SDL_SetRenderDrawColor(renderer, display_bg.r, display_bg.g, display_bg.b, 255);
    SDL_RenderFillRect(renderer, &background);

    // Render AMPM
    SDL_RenderCopy(renderer, inactive_ampm, NULL, &ampm_box);
    SDL_RenderCopy(renderer, active_ampm, NULL, &ampm_box);

    // Render inactive
    SDL_RenderCopy(renderer, inactive_texture_no_outline, NULL, &centered_textbox);
    SDL_RenderCopy(renderer, inactive_texture, NULL, &text_box);

    // Render active
    SDL_RenderCopy(renderer, active_texture_no_outline, NULL, &centered_textbox);
    SDL_RenderCopy(renderer, active_texture, NULL, &text_box);

    // free surfaces and textures
    SDL_FreeSurface(ampm);
    SDL_FreeSurface(inactive_surface);
    SDL_FreeSurface(ampm_surf);
    SDL_FreeSurface(active_surface);
    SDL_FreeSurface(active_surface_no_outline);
    SDL_FreeSurface(inactive_surface_no_outline);
    SDL_DestroyTexture(inactive_texture);
    SDL_DestroyTexture(inactive_ampm);
    SDL_DestroyTexture(active_ampm);
    SDL_DestroyTexture(active_texture);
    SDL_DestroyTexture(inactive_texture_no_outline);
    SDL_DestroyTexture(active_texture_no_outline);

    // Render Screen
    SDL_Rect video_screen = {50, 50, 320, 240};
    render_videoscreen(renderer, video_screen);
}

void standby_screen(SDL_Renderer *renderer, VcrColorPalette colors) {
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

    draw_standby_digital_display(renderer, 150, 300, 400, 50);

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
            SDL_snprintf(text, sizeof(text), "I found this (%d)", event->key.keysym.sym);
            SDL_Color foreground = {0x6E, 0xFB, 0x4C};
            SDL_Surface *text_surface = TTF_RenderText_Blended(smallfont, text, foreground);

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
            standby_screen(renderer, supercolor_palette());
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

    font = TTF_OpenFont("/usr/share/fonts/truetype/dseg/DSEG7ModernMini-Italic.ttf", 48);
    if (!font) {
        // TODO(ryan): Uh oh...
        printf("Failed to open font\n");
    }
    smallfont = TTF_OpenFont("/usr/share/fonts/truetype/dseg/DSEG14Classic-Regular.ttf", 24);
    alphafont = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/UbuntuSansMono[wght].ttf", 24);

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

#ifdef DEBUG
    window_x_pos = 5120 - 740;
    window_y_pos = 100;
#endif

    window = SDL_CreateWindow("vcr display", window_x_pos, window_y_pos, 640, 480, SDL_WINDOW_BORDERLESS);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    standby_screen(renderer, default_color_palette());
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
