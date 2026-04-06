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
    SDL_Rect inset_video_screen;
    SDL_Rect inset_info_container;
} VcrApplication;


int init_vcr_application(VcrApplication *vcr_display) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to init SDL\n");
        return 1;
    }

    if (TTF_Init()) {
        printf("Failed to init TTF\n");
        return 1;
    }

    vcr_display->font_digital_clock_7seg =
        TTF_OpenFont("/usr/share/fonts/truetype/dseg/DSEG7ModernMini-Italic.ttf", 48);
    if (!vcr_display->font_digital_clock_7seg) {
        printf("Failed to open clock font\n");
        return 1;
    }

    vcr_display->font_default = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 20);
    if (!vcr_display->font_default) {
        printf("Failed to open default font\n");
        return 1;
    }

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

#ifdef DEBUG
    window_x_pos = 5120 - 740;
    window_y_pos = 100;
#endif

    vcr_display->window = SDL_CreateWindow("VCR", window_x_pos, window_y_pos, 640, 480, SDL_WINDOW_BORDERLESS);
    vcr_display->renderer = SDL_CreateRenderer(vcr_display->window, -1, SDL_RENDERER_SOFTWARE);

    return 0;
}


void destroy_vcr_application(VcrApplication *vcr_display) {

    SDL_DestroyWindow(vcr_display->window);
    SDL_DestroyRenderer(vcr_display->renderer);
    TTF_CloseFont(vcr_display->font_digital_clock_7seg);
    TTF_CloseFont(vcr_display->font_default);

    vcr_display->window = NULL;
    vcr_display->renderer = NULL;
    vcr_display->font_digital_clock_7seg = NULL;
    vcr_display->font_default = NULL;

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

void render_visual_static(VcrApplication *vcr_display, SDL_Rect screen) {
    SDL_Texture *static_texture = SDL_CreateTexture(vcr_display->renderer, SDL_PIXELFORMAT_RGBA8888,
                                                    SDL_TEXTUREACCESS_STATIC, screen.w, screen.h);

    Uint32 pixels[screen.w * screen.h];
    for (int y = 0; y < screen.h; ++y) {
        for (int x = 0; x < screen.w; ++x) {
            Uint8 noise = (Uint8)rand();
            pixels[y * screen.w + x] = (noise << 24) | (noise << 16) | (noise << 8) | 0xff; // RGBA
        }
    }

    SDL_UpdateTexture(static_texture, NULL, pixels, sizeof(Uint32) * screen.w);
    SDL_RenderCopy(vcr_display->renderer, static_texture, NULL, &screen);
    SDL_DestroyTexture(static_texture);
}

void render_videoscreen(VcrApplication *vcr_display, SDL_Rect screen) {

    SDL_Rect shadow = screen;
    int shadow_size = 5;
    int shadow_alpha = 0xFF / 2;
    int alpha_decay = shadow_alpha / shadow_size;

    SDL_BlendMode reset_blend_mode;
    SDL_GetRenderDrawBlendMode(vcr_display->renderer, &reset_blend_mode);

    SDL_SetRenderDrawBlendMode(vcr_display->renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < shadow_size; i++) {
        shadow.x -= 1;
        shadow.y -= 1;
        shadow.w += 2;
        shadow.h += 2;
        shadow_alpha -= alpha_decay;
        SDL_SetRenderDrawColor(vcr_display->renderer, 0x00, 0x00, 0x00, shadow_alpha);
        SDL_RenderDrawRect(vcr_display->renderer, &shadow);
    }

    // render_visual_static(vcr_display, screen);
    SDL_SetRenderDrawBlendMode(vcr_display->renderer, reset_blend_mode);
    // SDL_SetRenderDrawColor(vcr_display->renderer, 0x1A, 0xFD, 0xD7, 0xFF);
    SDL_RenderFillRect(vcr_display->renderer, &screen);
}

SDL_Texture *render_digital_display(VcrApplication *vcr_display, DigitalDisplayState *display_state) {

    SDL_Texture *texture_inactive_clock;
    SDL_Texture *texture_inactive_channel;
    SDL_Texture *texture_active_clock;
    SDL_Texture *texture_active_channel;
    SDL_Texture *texture_symbol_pm;
    SDL_Texture *texture_symbol_am;
    SDL_Texture *texture_symbol_vcr;
    SDL_Texture *texture_symbol_rec;
    SDL_Texture *texture_symbol_hifi;
    SDL_Texture *texture_glyph_play;
    SDL_Texture *texture_glyph_ff;
    SDL_Texture *texture_glyph_rew;
    SDL_Texture *texture_pause_glyph;

    SDL_Color active_base_color     = PALETTE_LCD_BLUE_DIM;
    SDL_Color active_bright_color   = PALETTE_LCD_BLUE_BRIGHT;
    SDL_Color inactive_base_color   = PALETTE_LCD_INACTIVE_DIM;
    SDL_Color inactive_bright_color = PALETTE_LCD_INACTIVE_BRIGHT;

    char fmt_channel[3];
    char fmt_clock[6];
    SDL_snprintf(fmt_channel, sizeof(fmt_channel), "%02d", display_state->channel);
    SDL_snprintf(fmt_clock, sizeof(fmt_clock), "%02d:%02d", display_state->hour, display_state->minute);
    if (display_state->hour < 10) {
        fmt_clock[0] = *"!";
    }

    texture_inactive_clock = create_glow_text_two_layer(vcr_display->renderer, vcr_display->font_digital_clock_7seg, "88:88", inactive_base_color, inactive_bright_color, false, false);
    texture_inactive_channel = create_glow_text_two_layer(vcr_display->renderer, vcr_display->font_digital_clock_7seg, "88", inactive_base_color, inactive_bright_color, false, false);
    texture_active_clock = create_glow_text_two_layer(vcr_display->renderer, vcr_display->font_digital_clock_7seg, fmt_clock, active_base_color, active_bright_color, true, false);
    texture_active_channel = create_glow_text_two_layer(vcr_display->renderer, vcr_display->font_digital_clock_7seg, fmt_channel, active_base_color, active_bright_color, true, false);
    texture_symbol_pm = create_digital_display_symbol(vcr_display->renderer, vcr_display->font_default, " PM ", display_state->pm);
    texture_symbol_am = create_digital_display_symbol(vcr_display->renderer, vcr_display->font_default, " AM ", display_state->am);
    texture_symbol_vcr = create_digital_display_symbol(vcr_display->renderer, vcr_display->font_default, "VCR", display_state->vcr);
    texture_symbol_rec = create_digital_display_symbol(vcr_display->renderer, vcr_display->font_default, "REC", display_state->rec);
    texture_symbol_hifi = create_digital_display_symbol(vcr_display->renderer, vcr_display->font_default, "Hi-Fi", display_state->hifi);

    SDL_Rect container_clock;
    SDL_Rect container_channel;
    SDL_Rect container_pm;
    SDL_Rect container_am;
    SDL_Rect container_vcr;
    SDL_Rect container_rec;
    SDL_Rect container_hifi;

    SDL_QueryTexture(texture_inactive_clock, NULL, NULL, &container_clock.w, &container_clock.h);
    SDL_QueryTexture(texture_inactive_channel, NULL, NULL, &container_channel.w, &container_channel.h);
    SDL_QueryTexture(texture_symbol_pm, NULL, NULL, &container_pm.w, &container_pm.h);
    SDL_QueryTexture(texture_symbol_am, NULL, NULL, &container_am.w, &container_am.h);
    SDL_QueryTexture(texture_symbol_vcr, NULL, NULL, &container_vcr.w, &container_vcr.h);
    SDL_QueryTexture(texture_symbol_rec, NULL, NULL, &container_rec.w, &container_rec.h);
    SDL_QueryTexture(texture_symbol_hifi, NULL, NULL, &container_hifi.w, &container_hifi.h);

    int glyph_height = container_clock.h / 2;
    int play_glyph_width = (int)(glyph_height * 0.75);
    int pause_bar_glyph_width = glyph_height  / 2;
    int glyph_spacing = pause_bar_glyph_width;

    texture_glyph_play = create_play_arrow_glyph_texture(
        vcr_display->renderer,
        play_glyph_width,
        glyph_height,
        display_state->play || display_state->fast_forward
    );
    texture_glyph_ff = create_play_arrow_glyph_texture(
        vcr_display->renderer,
        play_glyph_width,
        glyph_height,
        display_state->fast_forward
    );
    texture_glyph_rew = create_play_arrow_glyph_texture(
        vcr_display->renderer,
        play_glyph_width,
        glyph_height,
        display_state->rewind
    );
    texture_pause_glyph = create_pause_bar_glyph(vcr_display->renderer, pause_bar_glyph_width, glyph_height, display_state->pause);

    int space_between_clock_and_channel = glyph_height;
    int top_row_width = container_clock.w + container_am.w + container_channel.w + space_between_clock_and_channel;
    int bottom_row_width = container_vcr.w + container_rec.w + container_hifi.w + (play_glyph_width * 4) + (pause_bar_glyph_width * 2) + (glyph_spacing * 5);
    int bottom_symbol_space = (top_row_width - bottom_row_width) / 3;
    int bottom_row_y = container_clock.h + 10;

    SDL_Rect container_rew1   = { 0, bottom_row_y,      play_glyph_width, glyph_height };
    SDL_Rect container_rew2   = { 0, bottom_row_y,      play_glyph_width, glyph_height };
    SDL_Rect container_pause1 = { 0, bottom_row_y, pause_bar_glyph_width, glyph_height };
    SDL_Rect container_pause2 = { 0, bottom_row_y, pause_bar_glyph_width, glyph_height };
    SDL_Rect container_play   = { 0, bottom_row_y,      play_glyph_width, glyph_height };
    SDL_Rect container_ff     = { 0, bottom_row_y,      play_glyph_width, glyph_height };

    container_clock.x = 0;
    container_clock.y = 0;
    container_am.x = container_clock.w;
    container_am.y = 0;
    container_pm.x = container_clock.w;
    container_pm.y = container_clock.h - container_pm.h;
    container_channel.x = container_clock.w + container_am.w + space_between_clock_and_channel;
    container_channel.y = 0;

    container_vcr.x = 0;
    container_vcr.y = bottom_row_y;
    container_rew1.x = container_vcr.w + bottom_symbol_space;
    container_rew2.x = container_rew1.x + play_glyph_width + glyph_spacing;
    container_pause1.x = container_rew2.x + play_glyph_width + glyph_spacing;
    container_pause2.x = container_pause1.x + pause_bar_glyph_width + glyph_spacing;
    container_play.x = container_pause2.x + pause_bar_glyph_width + glyph_spacing;
    container_ff.x = container_play.x + play_glyph_width + glyph_spacing;
    container_rec.x = container_ff.x + play_glyph_width + bottom_symbol_space;
    container_rec.y = bottom_row_y;
    container_hifi.x = container_rec.x + container_rec.w + bottom_symbol_space;
    container_hifi.y = bottom_row_y;

    SDL_Texture *target = SDL_CreateTexture(vcr_display->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, top_row_width, bottom_row_y + container_rec.h);
    SDL_SetRenderTarget(vcr_display->renderer, target);
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(vcr_display->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(vcr_display->renderer, NULL);

    SDL_RenderCopy(vcr_display->renderer, texture_inactive_clock, NULL, &container_clock);
    SDL_RenderCopy(vcr_display->renderer, texture_active_clock, NULL, &container_clock);
    SDL_RenderCopy(vcr_display->renderer, texture_symbol_am, NULL, &container_am);
    SDL_RenderCopy(vcr_display->renderer, texture_symbol_pm, NULL, &container_pm);
    SDL_RenderCopy(vcr_display->renderer, texture_inactive_channel, NULL, &container_channel);
    SDL_RenderCopy(vcr_display->renderer, texture_active_channel, NULL, &container_channel);
    SDL_RenderCopy(vcr_display->renderer, texture_symbol_vcr, NULL, &container_vcr);
    SDL_RenderCopy(vcr_display->renderer, texture_symbol_rec, NULL, &container_rec);
    SDL_RenderCopy(vcr_display->renderer, texture_symbol_hifi, NULL, &container_hifi);

    SDL_RenderCopyEx(vcr_display->renderer, texture_glyph_rew, NULL, &container_rew1, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopyEx(vcr_display->renderer, texture_glyph_rew, NULL, &container_rew2, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopy(vcr_display->renderer, texture_pause_glyph, NULL, &container_pause1);
    SDL_RenderCopy(vcr_display->renderer, texture_pause_glyph, NULL, &container_pause2);
    SDL_RenderCopy(vcr_display->renderer, texture_glyph_play, NULL, &container_play);
    SDL_RenderCopy(vcr_display->renderer, texture_glyph_ff, NULL, &container_ff);

    SDL_DestroyTexture(texture_inactive_clock);
    SDL_DestroyTexture(texture_inactive_channel);
    SDL_DestroyTexture(texture_active_clock);
    SDL_DestroyTexture(texture_active_channel);
    SDL_DestroyTexture(texture_symbol_pm);
    SDL_DestroyTexture(texture_symbol_am);
    SDL_DestroyTexture(texture_symbol_vcr);
    SDL_DestroyTexture(texture_symbol_rec);
    SDL_DestroyTexture(texture_symbol_hifi);
    SDL_DestroyTexture(texture_glyph_play);
    SDL_DestroyTexture(texture_glyph_ff);
    SDL_DestroyTexture(texture_glyph_rew);
    SDL_DestroyTexture(texture_pause_glyph);

    SDL_SetRenderTarget(vcr_display->renderer, NULL);

    return target;
}

void standby_screen(VcrApplication *vcr_display, VcrColorPalette colors, bool clear) {
    SDL_SetRenderDrawColor(vcr_display->renderer, colors.text_fg.r, colors.text_fg.g, colors.text_fg.b, 255);
    SDL_RenderClear(vcr_display->renderer);

    int padding = 125;
    int bar_height = (480 - padding * 2) / 5;

    SDL_Rect r0 = {0, padding + (bar_height * 0), 640, bar_height};
    SDL_Rect r1 = {0, padding + (bar_height * 1), 640, bar_height};
    SDL_Rect r2 = {0, padding + (bar_height * 2), 640, bar_height};
    SDL_Rect r3 = {0, padding + (bar_height * 3), 640, bar_height};
    SDL_Rect r4 = {0, padding + (bar_height * 4), 640, bar_height};

    SDL_SetRenderDrawColor(vcr_display->renderer, colors.c0.r, colors.c0.g, colors.c0.b, 255);
    SDL_RenderFillRect(vcr_display->renderer, &r0);

    SDL_SetRenderDrawColor(vcr_display->renderer, colors.c1.r, colors.c1.g, colors.c1.b, 255);
    SDL_RenderFillRect(vcr_display->renderer, &r1);

    SDL_SetRenderDrawColor(vcr_display->renderer, colors.c2.r, colors.c2.g, colors.c2.b, 255);
    SDL_RenderFillRect(vcr_display->renderer, &r2);

    SDL_SetRenderDrawColor(vcr_display->renderer, colors.c3.r, colors.c3.g, colors.c3.b, 255);
    SDL_RenderFillRect(vcr_display->renderer, &r3);

    SDL_SetRenderDrawColor(vcr_display->renderer, colors.c4.r, colors.c4.g, colors.c4.b, 255);
    SDL_RenderFillRect(vcr_display->renderer, &r4);

    SDL_RenderPresent(vcr_display->renderer);
    if (clear) {
        SDL_RenderClear(vcr_display->renderer);
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

bool handle_event(SDL_Event *event, VcrApplication *vcr_display) {

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

        if (!vcr_display->font_digital_clock_7seg) {
            r = event->key.keysym.sym % 255;
            g = (event->key.keysym.sym * 2) % 255;
            b = (event->key.keysym.sym * event->key.keysym.sym) % 255;
            SDL_SetRenderDrawColor(vcr_display->renderer, r, g, b, 255);
            SDL_RenderClear(vcr_display->renderer);
        } else {
            SDL_SetRenderDrawColor(vcr_display->renderer, 0, 0, 0, 255);
            SDL_RenderClear(vcr_display->renderer);
            char text[50];
            SDL_snprintf(text, sizeof(text), "I found this (%d)", event->key.keysym.sym);
            SDL_Color foreground = {0x6E, 0xFB, 0x4C};

            SDL_Surface *text_surface = TTF_RenderText_Blended(vcr_display->font_default, text, foreground);
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(vcr_display->renderer, text_surface);

            float scale = 640.0 / text_surface->w;
            SDL_Rect dest = {0, 0, 640, (int)(text_surface->h * scale)};
            SDL_RenderCopy(vcr_display->renderer, text_texture, NULL, &dest);

            SDL_DestroyTexture(text_texture);
            SDL_FreeSurface(text_surface);
        }

        SDL_RenderPresent(vcr_display->renderer);
        should_quit = process_key_stroke(event->key.keysym);
        SDL_RenderClear(vcr_display->renderer);

        if (event->key.keysym.sym == SDLK_s) {
            standby_screen(vcr_display, supercolor_palette(), false);
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

int run_display_loop(void) {

    VcrApplication vcr_display;
    if (init_vcr_application(&vcr_display) != 0) {
        printf("Failed to init vcr display\n");
    }

    DigitalDisplayState display_state = {
        2,
        0,
        4,
        false,
        true,
        true,
        true,
        false,
        true,
        true,
        false,
        false,
    };

    SDL_Rect video_screen = {50, 50, 320, 240};
    const int TARGET_FPS = 24;
    const int FRAME_DELAY = 1000 / TARGET_FPS;

    standby_screen(&vcr_display, default_color_palette(), false);
    // render_videoscreen(&vcr_display, video_screen);
     
    SDL_Texture *tex = render_digital_display(&vcr_display, &display_state);
    SDL_Rect digital_display_container = { 150, 300, 0, 0 };
    SDL_QueryTexture(tex, NULL, NULL, &digital_display_container.w, &digital_display_container.h);


    display_state = (DigitalDisplayState){
        88,
        88,
        88,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
    };
    SDL_Texture *tex2 = render_digital_display(&vcr_display, &display_state);

    SDL_Rect digi_inactive = { 150, 300, 0, 0 };
    SDL_Rect digi_active = { 150, 300, 0, 0 };
    SDL_Rect digi_drawn = { 150, 300, 0, 0 };
    SDL_QueryTexture(tex2, NULL, NULL, &digi_active.w, &digi_active.h);
    DigitalDisplay *digi = create_digital_display(vcr_display.renderer, vcr_display.font_digital_clock_7seg, vcr_display.font_default);

    digi_drawn.y = digital_display_container.y - (5 + digital_display_container.h);
    digi_drawn.w = digi->container.w;
    digi_drawn.h = digi->container.h;
    digi_active.y = digi_drawn.y - (5 + digi_drawn.h);
    digi_active.w = digi->container.w;
    digi_active.h = digi->container.h;
    digi_inactive.y = digi_active.y - (5 + digi_active.h);
    digi_inactive.w = digi->container.w;
    digi_inactive.h = digi->container.h;

    SDL_RenderCopy(vcr_display.renderer, tex2, NULL, &digi_active);
    SDL_RenderCopy(vcr_display.renderer, digi->inactive, NULL, &digi_inactive);
    SDL_RenderCopy(vcr_display.renderer, digi->active, NULL, &digi_inactive);

    display_state = (DigitalDisplayState){
        2,
        0,
        4,
        false,
        true,
        true,
        true,
        false,
        true,
        true,
        false,
        false,
    };
    display_state.fast_forward = false;
    display_state.play = true;
    digi->container.x = digi_drawn.x;
    digi->container.y = digi_drawn.y;
    draw_digital_display(vcr_display.renderer, vcr_display.font_digital_clock_7seg, digi, &display_state);

    SDL_RenderCopy(vcr_display.renderer, tex, NULL, &digital_display_container);
    SDL_DestroyTexture(tex);

    bool running = true;
    while (running) {

        int frame_start = SDL_GetTicks();
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {

            if (handle_event(&Event, &vcr_display)) {
                running = false;
            }
        }

        // render_visual_static(&vcr_display, video_screen);
        SDL_RenderPresent(vcr_display.renderer);

        int frame_time = SDL_GetTicks() - frame_start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    destroy_vcr_application(&vcr_display);
    return (0);
}
