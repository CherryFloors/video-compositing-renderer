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
#include <mpv/client.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define VCR_ASSETS_IMPLEMENTATION
#include "vcr_assets.h"
#define VIDEO_PLAYER_IMPLEMENTATION
#include "video_player.h"

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
    VideoPlayerContext video_player;
} VcrApplication;

typedef enum VcrEvent {
    VCR_EVENT_NONE      = 0,
    VCR_EVENT_QUIT      = 1,
    VCR_EVENT_VIDEO_END = 2,
} VcrEvent;

int init_vcr_application(VcrApplication *vcr_app) {

    // TODO: Do I need this?
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");

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

    init_video_player(&vcr_app->video_player, vcr_app->renderer, vcr_app->video_screen.w, vcr_app->video_screen.h);

    return 0;
}


void destroy_vcr_application(VcrApplication *vcr_app) {

    SDL_DestroyWindow(vcr_app->window);
    SDL_DestroyRenderer(vcr_app->renderer);
    TTF_CloseFont(vcr_app->font_digital_clock_7seg);
    TTF_CloseFont(vcr_app->font_default);
    destroy_digital_display(vcr_app->digital_display);
    destroy_video_player(&vcr_app->video_player);

    vcr_app->window = NULL;
    vcr_app->renderer = NULL;
    vcr_app->font_digital_clock_7seg = NULL;
    vcr_app->font_default = NULL;
    vcr_app->digital_display = NULL;

    TTF_Quit();
    SDL_Quit();
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

void render_videoscreen_shadow(VcrApplication *vcr_app, SDL_Rect screen) {

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

    SDL_SetRenderDrawBlendMode(vcr_app->renderer, reset_blend_mode);
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

VcrEvent process_key_stroke(VcrApplication *vcr_app, SDL_Keysym symbol) {

    VcrEvent processed_keystroke = VCR_EVENT_NONE;
    switch (symbol.sym) {
        case SDLK_q:
            processed_keystroke = VCR_EVENT_QUIT;
            break;
        case SDLK_s:
            standby_screen(vcr_app, supercolor_palette(), false);
            break;
        case SDLK_d:
            standby_screen(vcr_app, default_color_palette(), false);
            break;
        default:
            processed_keystroke = VCR_EVENT_NONE;
            printf("I found this [%d]\n", symbol.sym);
            break;
    }

    return processed_keystroke;
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

VcrEvent process_default_sdl_events(VcrApplication *vcr_app, SDL_Event *event) {

    VcrEvent default_event = VCR_EVENT_NONE;
    if(event->type == vcr_app->video_player.event_wakeup_on_mpv_render_update) {
        render_video_frame(&vcr_app->video_player, vcr_app->renderer, &vcr_app->video_screen);
    }

    if (event->type == vcr_app->video_player.event_wakeup_on_mpv_events) {
        // Handle all remaining mpv events.
        while (1) {
            mpv_event *mp_event = mpv_wait_event(vcr_app->video_player.mpv, 0);
            if (mp_event->event_id == MPV_EVENT_NONE)
                break;
            if (mp_event->event_id == MPV_EVENT_END_FILE) {
                return VCR_EVENT_VIDEO_END;
            }
            if (mp_event->event_id == MPV_EVENT_LOG_MESSAGE) {
                mpv_event_log_message *msg = mp_event->data;
                // Print log messages about DR allocations, just to
                // test whether it works. If there is more than 1 of
                // these, it works. (The log message can actually change
                // any time, so it's possible this logging stops working
                // in the future.)
                if (strstr(msg->text, "DR image"))
                    printf("log: %s", msg->text);
                continue;
            }
            printf("event: %s\n", mpv_event_name(mp_event->event_id));
        }
    }

    return default_event;
}

VcrEvent process_sdl_event(VcrApplication *vcr_app, SDL_Event *event) {

    VcrEvent processed_event;
    switch (event->type) {
        case SDL_QUIT:
            processed_event = VCR_EVENT_QUIT;
            break;
        case SDL_KEYDOWN:
            processed_event = process_key_stroke(vcr_app, event->key.keysym);
            break;
        default:
            processed_event = process_default_sdl_events(vcr_app, event);
            break;
    }
    return processed_event;
}

// TODO: Rename to start_engine(program_q)
int run_display_loop(void) {

    // char *f1 = "";

    VcrApplication vcr_app;
    if (init_vcr_application(&vcr_app) != 0) {
        printf("Failed to init vcr display\n");
    }

    const int TARGET_FPS = 24;
    const int FRAME_DELAY = 1000 / TARGET_FPS;

    standby_screen(&vcr_app, default_color_palette(), false);
    render_videoscreen_shadow(&vcr_app, vcr_app.video_screen);
    bool redraw_digital_display = update_digital_clock_and_check_for_redraw(&vcr_app.digital_display_state);
    draw_digital_display(vcr_app.renderer, vcr_app.font_digital_clock_7seg, vcr_app.digital_display, &vcr_app.digital_display_state);

    // play_file(&vcr_app.video_player, f1);

    bool running = true;
    bool play_2 = true;
    while (running) {

        int frame_start = SDL_GetTicks();
        SDL_Event sdl_event;
        VcrEvent vcr_event;
        while (SDL_PollEvent(&sdl_event)) {

            vcr_event = process_sdl_event(&vcr_app, &sdl_event);
            if (vcr_event == VCR_EVENT_QUIT) {
                running = false;
            }

            if (vcr_event == VCR_EVENT_VIDEO_END && play_2) {
                play_2 = false;
                // play_file(&vcr_app.video_player, f2);
            }
        }

        render_video_static(&vcr_app.video_player, vcr_app.renderer, &vcr_app.video_screen);
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
    return 0;
}
