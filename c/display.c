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

typedef struct DigitalDisplayState {
    int hour;
    int minute;
    int channel;
    bool am;
    bool vcr;
    bool hifi;
    bool rec;
    bool pause;
    bool play;
    bool fast_forward;
    bool rewind;
} DigitalDisplayState;

typedef struct VcrDisplay {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *digital_clock_font;
    TTF_Font *default_font;
} VcrDisplay;

int init_vcr_display_and_sdl(VcrDisplay *vcr_display) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to init SDL\n");
        return 1;
    }

    if (TTF_Init()) {
        printf("Failed to init TTF\n");
        return 1;
    }

    vcr_display->digital_clock_font = TTF_OpenFont("/usr/share/fonts/truetype/dseg/DSEG7ModernMini-Italic.ttf", 48);
    if (!vcr_display->digital_clock_font) {
        printf("Failed to open clock font\n");
    }

    vcr_display->default_font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 24);
    if (!vcr_display->default_font) {
        printf("Failed to open default font\n");
    }

    int window_x_pos = SDL_WINDOWPOS_UNDEFINED;
    int window_y_pos = SDL_WINDOWPOS_UNDEFINED;

#ifdef DEBUG
    window_x_pos = 5120 - 740;
    window_y_pos = 100;
#endif

    vcr_display->window = SDL_CreateWindow("vcr display", window_x_pos, window_y_pos, 640, 480, SDL_WINDOW_BORDERLESS);
    vcr_display->renderer = SDL_CreateRenderer(vcr_display->window, -1, SDL_RENDERER_SOFTWARE);

    return 0;
}

void destroy_vcr_display_and_clean_up_sdl(VcrDisplay *vcr_display) {

    SDL_DestroyWindow(vcr_display->window);
    SDL_DestroyRenderer(vcr_display->renderer);
    TTF_CloseFont(vcr_display->digital_clock_font);
    TTF_CloseFont(vcr_display->default_font);

    vcr_display->window = NULL;
    vcr_display->renderer = NULL;
    vcr_display->digital_clock_font = NULL;
    vcr_display->default_font = NULL;

    TTF_Quit();
    SDL_Quit();
}

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

void render_visual_static(VcrDisplay *vcr_display, SDL_Rect screen) {
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

void render_videoscreen(VcrDisplay *vcr_display, SDL_Rect screen) {

    SDL_Rect shadow = screen;
    int shadow_size = 5;
    int shadow_alpha = 0xFF / 2;
    int alpha_decay = shadow_alpha / shadow_size;

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

    render_visual_static(vcr_display, screen);
    // SDL_SetRenderDrawColor(renderer, 0x1A, 0xFD, 0xD7, 0xFF);
    // SDL_RenderFillRect(renderer, &screen);
}

void draw_standby_digital_display(VcrDisplay *vcr_display, int x, int y, int w, int h) {

    int outw = 1;
    TTF_SetFontOutline(vcr_display->digital_clock_font, outw);
    TTF_SetFontOutline(vcr_display->default_font, outw);

    SDL_Color display_bg = {0x00, 0x00, 0x00};
    SDL_Color solid_text_color = {0x1A, 0xFD, 0xD7};
    SDL_Color active_text_outline_color = {solid_text_color.r, solid_text_color.g, solid_text_color.b, 120};
    SDL_Color active_text_center_color = {0xBD, 0xFF, 0xFD};
    SDL_Color inactive_text_outline_color = {0x3A, 0x3A, 0x3A};
    SDL_Color inactive_text_center_color = {0x25, 0x25, 0x25};

    SDL_Surface *ampm = TTF_RenderText_Blended(vcr_display->default_font, " PM", active_text_outline_color);
    SDL_Surface *inactive_surface =
        TTF_RenderText_Blended(vcr_display->digital_clock_font, "8888888:88", inactive_text_outline_color);
    SDL_Surface *ampm_surf = TTF_RenderText_Blended(vcr_display->default_font, " PM", active_text_center_color);
    SDL_Surface *active_surface =
        TTF_RenderText_Blended(vcr_display->digital_clock_font, "VCR!!12:34", active_text_outline_color);
    TTF_SetFontOutline(vcr_display->default_font, 0);
    TTF_SetFontOutline(vcr_display->digital_clock_font, 0);
    SDL_Surface *active_surface_no_outline =
        TTF_RenderText_Blended(vcr_display->digital_clock_font, "VCR!!12:34", active_text_center_color);
    SDL_Surface *inactive_surface_no_outline =
        TTF_RenderText_Blended(vcr_display->digital_clock_font, "8888888:88", inactive_text_center_color);

    int padding = w - inactive_surface->w;
    x = (640 - w + ampm->w + padding) / 2;
    SDL_Rect background = {x, y, w + ampm->w, inactive_surface->h + padding};
    SDL_Rect text_box = {x + padding / 2, y + padding / 2, inactive_surface->w, inactive_surface->h};
    SDL_Rect ampm_box = {text_box.x + text_box.w, (text_box.h - ampm->h) + text_box.y, ampm->w, ampm->h};
    SDL_Rect centered_textbox = {text_box.x + outw, text_box.y + outw, inactive_surface_no_outline->w,
                                 inactive_surface_no_outline->h};

    SDL_Texture *inactive_texture = SDL_CreateTextureFromSurface(vcr_display->renderer, inactive_surface);
    SDL_Texture *inactive_ampm = SDL_CreateTextureFromSurface(vcr_display->renderer, ampm);
    SDL_Texture *active_ampm = SDL_CreateTextureFromSurface(vcr_display->renderer, ampm_surf);
    SDL_Texture *active_texture = SDL_CreateTextureFromSurface(vcr_display->renderer, active_surface);
    SDL_Texture *inactive_texture_no_outline =
        SDL_CreateTextureFromSurface(vcr_display->renderer, inactive_surface_no_outline);
    SDL_Texture *active_texture_no_outline =
        SDL_CreateTextureFromSurface(vcr_display->renderer, active_surface_no_outline);

    // display bg
    SDL_SetRenderDrawColor(vcr_display->renderer, display_bg.r, display_bg.g, display_bg.b, 255);
    SDL_RenderFillRect(vcr_display->renderer, &background);

    // Render AMPM
    SDL_RenderCopy(vcr_display->renderer, inactive_ampm, NULL, &ampm_box);
    SDL_RenderCopy(vcr_display->renderer, active_ampm, NULL, &ampm_box);

    // Render inactive
    SDL_RenderCopy(vcr_display->renderer, inactive_texture_no_outline, NULL, &centered_textbox);
    SDL_RenderCopy(vcr_display->renderer, inactive_texture, NULL, &text_box);

    // Render active
    SDL_RenderCopy(vcr_display->renderer, active_texture_no_outline, NULL, &centered_textbox);
    SDL_RenderCopy(vcr_display->renderer, active_texture, NULL, &text_box);

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
    render_videoscreen(vcr_display, video_screen);
}

void standby_screen(VcrDisplay *vcr_display, VcrColorPalette colors, bool clear) {
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

    draw_standby_digital_display(vcr_display, 150, 300, 400, 50);

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

bool handle_event(SDL_Event *event, VcrDisplay *vcr_display) {

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

        if (!vcr_display->digital_clock_font) {
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

            SDL_Surface *text_surface = TTF_RenderText_Blended(vcr_display->default_font, text, foreground);
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

    VcrDisplay vcr_display;
    if (init_vcr_display_and_sdl(&vcr_display) != 0) {
        printf("Failed to init vcr display\n");
    }

    SDL_Rect video_screen = {50, 50, 320, 240};
    const int TARGET_FPS = 24;
    const int FRAME_DELAY = 1000 / TARGET_FPS;

    standby_screen(&vcr_display, default_color_palette(), false);
    bool running = true;
    while (running) {

        int frame_start = SDL_GetTicks();
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {

            if (handle_event(&Event, &vcr_display)) {
                running = false;
            }
        }

        render_visual_static(&vcr_display, video_screen);
        SDL_RenderPresent(vcr_display.renderer);

        int frame_time = SDL_GetTicks() - frame_start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
            printf("frame_time=%d  SDL_Delay(%d)\n", frame_time, FRAME_DELAY - frame_time);
        }
    }

    destroy_vcr_display_and_clean_up_sdl(&vcr_display);
    return (0);
}
