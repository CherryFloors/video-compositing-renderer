#pragma once
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
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define VCR_ASSETS_IMPLEMENTATION
#include "vcr_assets.h"


typedef enum DisplayResolution {
    RESOULUTION_SD_640_480,
    RESOULUTION_FHD_1920_1080,
} DisplayResolution;


typedef struct VcrApplication {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font_digital_clock_7seg;
    TTF_Font *font_default;
    SDL_Rect video_screen;
    SDL_Rect info_container;
    DigitalDisplay *digital_display;
    DigitalDisplayState digital_display_state;
} VcrApplication;


int init_vcr_application(VcrApplication *vcr_app) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to init SDL\n");
        return 1;
    }

    if (TTF_Init()) {
        printf("Failed to init TTF\n");
        return 1;
    }

    vcr_app->font_digital_clock_7seg =
        TTF_OpenFont("/usr/share/fonts/truetype/dseg/DSEG7ModernMini-Italic.ttf", 48);
    if (!vcr_app->font_digital_clock_7seg) {
        printf("Failed to open clock font\n");
        return 1;
    }

    vcr_app->font_default = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 20);
    if (!vcr_app->font_default) {
        printf("Failed to open default font\n");
        return 1;
    }

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

#ifdef DEBUG
    window_x_pos = 5120 - 740;
    window_y_pos = 100;
#endif

    vcr_app->video_screen = (SDL_Rect){50, 50, 320, 240};

    vcr_app->window = SDL_CreateWindow("VCR", window_x_pos, window_y_pos, 640, 480, SDL_WINDOW_BORDERLESS);
    vcr_app->renderer = SDL_CreateRenderer(vcr_app->window, -1, SDL_RENDERER_SOFTWARE);
    vcr_app->digital_display = create_digital_display(vcr_app->renderer, vcr_app->font_digital_clock_7seg, vcr_app->font_default);

    vcr_app->digital_display->container.x = 150;
    vcr_app->digital_display->container.y = 300;

    vcr_app->digital_display_state = (DigitalDisplayState){
        0, 0, 3, false, false, true, false, false, false, false, false, false,
    };

    return 0;
}


void destroy_vcr_application(VcrApplication *vcr_app) {

    SDL_DestroyWindow(vcr_app->window);
    SDL_DestroyRenderer(vcr_app->renderer);
    TTF_CloseFont(vcr_app->font_digital_clock_7seg);
    TTF_CloseFont(vcr_app->font_default);
    destroy_digital_display(vcr_app->digital_display);

    vcr_app->window = NULL;
    vcr_app->renderer = NULL;
    vcr_app->font_digital_clock_7seg = NULL;
    vcr_app->font_default = NULL;
    vcr_app->digital_display = NULL;

    TTF_Quit();
    SDL_Quit();
}

VcrColorPalette default_color_palette(void) {
    VcrColorPalette vcr_color_palette = {
        PALETTE_DYNAMICRON_YELLOW,
        PALETTE_DYNAMICRON_ORANGE,
        PALETTE_DYNAMICRON_RED,
        PALETTE_DYNAMICRON_DARK_RED,
        PALETTE_DYNAMICRON_PURPLE,
        PALETTE_DYNAMICRON_WHITE,
        PALETTE_DYNAMICRON_YELLOW,
    };
    return vcr_color_palette;
}

VcrColorPalette supercolor_palette(void) {
    VcrColorPalette vcr_color_palette = {
        PALETTE_SUPERCOLOR_BLUE,
        PALETTE_SUPERCOLOR_GREEN,
        PALETTE_SUPERCOLOR_YELLOW,
        PALETTE_SUPERCOLOR_ORANGE,
        PALETTE_SUPERCOLOR_RED,
        PALETTE_SUPERCOLOR_WHITE,
        PALETTE_SUPERCOLOR_BLACK,
    };
    return vcr_color_palette;
}

void render_visual_static(VcrApplication *vcr_app, SDL_Rect screen) {
    SDL_Texture *static_texture = SDL_CreateTexture(vcr_app->renderer, SDL_PIXELFORMAT_RGBA8888,
                                                    SDL_TEXTUREACCESS_STATIC, screen.w, screen.h);

    Uint32 pixels[screen.w * screen.h];
    for (int y = 0; y < screen.h; ++y) {
        for (int x = 0; x < screen.w; ++x) {
            Uint8 noise = (Uint8)rand();
            pixels[y * screen.w + x] = (noise << 24) | (noise << 16) | (noise << 8) | 0xff; // RGBA
        }
    }

    SDL_UpdateTexture(static_texture, NULL, pixels, sizeof(Uint32) * screen.w);
    SDL_RenderCopy(vcr_app->renderer, static_texture, NULL, &screen);
    SDL_DestroyTexture(static_texture);
}

void render_videoscreen(VcrApplication *vcr_app, SDL_Rect screen) {

    SDL_Rect shadow = screen;
    int shadow_size = 5;
    int shadow_alpha = 0xFF / 2;
    int alpha_decay = shadow_alpha / shadow_size;

    SDL_BlendMode reset_blend_mode;
    SDL_GetRenderDrawBlendMode(vcr_app->renderer, &reset_blend_mode);

    SDL_SetRenderDrawBlendMode(vcr_app->renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < shadow_size; i++) {
        shadow.x -= 1;
        shadow.y -= 1;
        shadow.w += 2;
        shadow.h += 2;
        shadow_alpha -= alpha_decay;
        SDL_SetRenderDrawColor(vcr_app->renderer, 0x00, 0x00, 0x00, shadow_alpha);
        SDL_RenderDrawRect(vcr_app->renderer, &shadow);
    }

    // render_visual_static(vcr_display, screen);
    SDL_SetRenderDrawBlendMode(vcr_app->renderer, reset_blend_mode);
    // SDL_SetRenderDrawColor(vcr_display->renderer, 0x1A, 0xFD, 0xD7, 0xFF);
    SDL_RenderFillRect(vcr_app->renderer, &screen);
}

void standby_screen(VcrApplication *vcr_app, VcrColorPalette colors, bool clear) {
    SDL_SetRenderDrawColor(vcr_app->renderer, colors.text_fg.r, colors.text_fg.g, colors.text_fg.b, 255);
    SDL_RenderClear(vcr_app->renderer);

    int padding = 125;
    int bar_height = (480 - padding * 2) / 5;

    SDL_Rect r0 = {0, padding + (bar_height * 0), 640, bar_height};
    SDL_Rect r1 = {0, padding + (bar_height * 1), 640, bar_height};
    SDL_Rect r2 = {0, padding + (bar_height * 2), 640, bar_height};
    SDL_Rect r3 = {0, padding + (bar_height * 3), 640, bar_height};
    SDL_Rect r4 = {0, padding + (bar_height * 4), 640, bar_height};

    SDL_SetRenderDrawColor(vcr_app->renderer, colors.c0.r, colors.c0.g, colors.c0.b, 255);
    SDL_RenderFillRect(vcr_app->renderer, &r0);

    SDL_SetRenderDrawColor(vcr_app->renderer, colors.c1.r, colors.c1.g, colors.c1.b, 255);
    SDL_RenderFillRect(vcr_app->renderer, &r1);

    SDL_SetRenderDrawColor(vcr_app->renderer, colors.c2.r, colors.c2.g, colors.c2.b, 255);
    SDL_RenderFillRect(vcr_app->renderer, &r2);

    SDL_SetRenderDrawColor(vcr_app->renderer, colors.c3.r, colors.c3.g, colors.c3.b, 255);
    SDL_RenderFillRect(vcr_app->renderer, &r3);

    SDL_SetRenderDrawColor(vcr_app->renderer, colors.c4.r, colors.c4.g, colors.c4.b, 255);
    SDL_RenderFillRect(vcr_app->renderer, &r4);

    SDL_RenderPresent(vcr_app->renderer);
    if (clear) {
        SDL_RenderClear(vcr_app->renderer);
    }
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

bool handle_event(SDL_Event *event, VcrApplication *vcr_app) {

    bool should_quit = false;
    int r, g, b;

    switch (event->type) {
    case SDL_QUIT:
        should_quit = true;
        break;

    case SDL_KEYDOWN:
        struct timeval t1, t2;
        double elapsed_time;
        gettimeofday(&t1, NULL);

        if (!vcr_app->font_digital_clock_7seg) {
            r = event->key.keysym.sym % 255;
            g = (event->key.keysym.sym * 2) % 255;
            b = (event->key.keysym.sym * event->key.keysym.sym) % 255;
            SDL_SetRenderDrawColor(vcr_app->renderer, r, g, b, 255);
            SDL_RenderClear(vcr_app->renderer);
        } else {
            SDL_SetRenderDrawColor(vcr_app->renderer, 0, 0, 0, 255);
            SDL_RenderClear(vcr_app->renderer);
            char text[50];
            SDL_snprintf(text, sizeof(text), "I found this (%d)", event->key.keysym.sym);
            SDL_Color foreground = {0x6E, 0xFB, 0x4C};

            SDL_Surface *text_surface = TTF_RenderText_Blended(vcr_app->font_default, text, foreground);
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(vcr_app->renderer, text_surface);

            float scale = 640.0 / text_surface->w;
            SDL_Rect dest = {0, 0, 640, (int)(text_surface->h * scale)};
            SDL_RenderCopy(vcr_app->renderer, text_texture, NULL, &dest);

            SDL_DestroyTexture(text_texture);
            SDL_FreeSurface(text_surface);
        }

        SDL_RenderPresent(vcr_app->renderer);
        should_quit = process_key_stroke(event->key.keysym);
        SDL_RenderClear(vcr_app->renderer);

        if (event->key.keysym.sym == SDLK_s) {
            standby_screen(vcr_app, supercolor_palette(), false);
        }
        gettimeofday(&t2, NULL);
        // Seconds to milliseconds
        elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000.0;
        // Microseconds to milliseconds
        elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000.0;
        printf("%f ms/frame  |  %f fps\n", elapsed_time, 1000.0 / elapsed_time);

        break;
    }

    return (should_quit);
}

void set_clock_hour_and_am_pm_from_24_hour_fmt(int hour_24_fmt, DigitalDisplayState *digital_display_state) {

    int hour = hour_24_fmt;
    digital_display_state->am = true;
    digital_display_state->pm = false;

    if (hour >= 12) {
        hour = hour - 12;
        digital_display_state->am = false;
        digital_display_state->pm = true;
    }

    if (hour == 0) {
        hour = 12;
    }

    digital_display_state->hour = hour;
}

bool update_digital_clock_and_check_for_redraw(DigitalDisplayState *digital_display_state) {

    bool redraw = false;
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    if (current_time->tm_hour != digital_display_state->hour) {
        redraw = true;
        set_clock_hour_and_am_pm_from_24_hour_fmt(current_time->tm_hour, digital_display_state);
    }

    if (current_time->tm_min != digital_display_state->minute) {
        redraw = true;
        digital_display_state->minute = current_time->tm_min;
    }

    return redraw;
}

int run_display_loop(void) {

    VcrApplication vcr_app;
    if (init_vcr_application(&vcr_app) != 0) {
        printf("Failed to init vcr display\n");
    }

    const int TARGET_FPS = 24;
    const int FRAME_DELAY = 1000 / TARGET_FPS;

    standby_screen(&vcr_app, default_color_palette(), false);
    render_videoscreen(&vcr_app, vcr_app.video_screen);
    bool redraw_digital_display = update_digital_clock_and_check_for_redraw(&vcr_app.digital_display_state);
    draw_digital_display(vcr_app.renderer, vcr_app.font_digital_clock_7seg, vcr_app.digital_display, &vcr_app.digital_display_state);

    bool running = true;
    while (running) {

        int frame_start = SDL_GetTicks();
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {

            if (handle_event(&Event, &vcr_app)) {
                running = false;
            }
        }

        render_visual_static(&vcr_app, vcr_app.video_screen);
        SDL_RenderPresent(vcr_app.renderer);

        // TODO: dont check this 24 times a second...
        redraw_digital_display = update_digital_clock_and_check_for_redraw(&vcr_app.digital_display_state);
        if (redraw_digital_display) {
            draw_digital_display(vcr_app.renderer, vcr_app.font_digital_clock_7seg, vcr_app.digital_display, &vcr_app.digital_display_state);
        }

        int frame_time = SDL_GetTicks() - frame_start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    destroy_vcr_application(&vcr_app);
    return (0);
}
