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
#include <unistd.h>

#include "engine.h"
#include "vcr_assets.h"
#include "video_player.h"
#include "vcr_programming_queue.h"

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

int init_vcr_application(VcrApplication *vcr_app) {

    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1"); // TODO: Do I need this?
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to init SDL\n");
        return 1;
    }

    if (TTF_Init()) {
        printf("Failed to init TTF\n");
        return 1;
    }

    vcr_app->font_digital_clock_7seg = TTF_OpenFont("/usr/share/fonts/truetype/dseg/DSEG7ModernMini-Italic.ttf", 48);  // TODO(cf): Font size scale and embedd
    if (!vcr_app->font_digital_clock_7seg) {
        printf("Failed to open clock font\n");
        return 1;
    }

    vcr_app->font_default = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 20);  // TODO(cf): Font size scale and embedd
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

    vcr_app->window = SDL_CreateWindow("VCR", window_x_pos, window_y_pos, RES_SD_W, RES_SD_H, SDL_WINDOW_BORDERLESS);
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

void standby_screen(VcrApplication *vcr_app, VcrColorPalette colors, bool clear) {  // TODO(cf): This belongs in assets... I think.
    SDL_SetRenderDrawColor(vcr_app->renderer, colors.text_fg.r, colors.text_fg.g, colors.text_fg.b, 255);
    SDL_RenderClear(vcr_app->renderer);

    int window_width, window_height;
    SDL_GetWindowSize(vcr_app->window, &window_width, &window_height);

    int padding = 190 * (RES_SD_H / window_height); 
    int bar_height = (window_height - padding * 2) / 5;

    SDL_Rect r0 = {0, padding + (bar_height * 0), window_width, bar_height};
    SDL_Rect r1 = {0, padding + (bar_height * 1), window_width, bar_height};
    SDL_Rect r2 = {0, padding + (bar_height * 2), window_width, bar_height};
    SDL_Rect r3 = {0, padding + (bar_height * 3), window_width, bar_height};
    SDL_Rect r4 = {0, padding + (bar_height * 4), window_width, bar_height};

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

VcrEvent clear_mpv_events(VcrApplication *vcr_app) {

    VcrEvent vcr_event = VCR_EVENT_NONE;
    while (true) {

        mpv_event *event_mpv = mpv_wait_event(vcr_app->video_player.mpv, 0);
        if (event_mpv->event_id == MPV_EVENT_NONE) {
            break;
        }

        if (event_mpv->event_id == MPV_EVENT_END_FILE) {
            vcr_event = VCR_EVENT_VIDEO_END;
        }

        if (event_mpv->event_id == MPV_EVENT_LOG_MESSAGE) {
            // mpv_event_log_message *msg = event_mpv->data;
            // printf("[MPV] %s", msg->text);
            continue;
        }
    }

    return vcr_event;

}

VcrEvent process_default_sdl_events(VcrApplication *vcr_app, SDL_Event *event) {

    VcrEvent default_event = VCR_EVENT_NONE;
    if (event->type == vcr_app->video_player.event_wakeup_on_mpv_render_update) {
        render_video_frame(&vcr_app->video_player, vcr_app->renderer, &vcr_app->video_screen);  // TODO(cf): handle bad render?
    }

    if (event->type == vcr_app->video_player.event_wakeup_on_mpv_events) {
        default_event = clear_mpv_events(vcr_app);
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

VcrEvent engine_routine_fullscreen_video(VcrApplication *vcr_app, VcrProgram *program) {

    SDL_Rect original_screen = vcr_app->video_screen;
    bool video_playing = true;
    int window_width, window_height;
    SDL_GetWindowSize(vcr_app->window, &window_width, &window_height);
    resize_screen(&vcr_app->video_player, vcr_app->renderer, window_width, window_height);

    vcr_app->video_screen.x = 0;
    vcr_app->video_screen.y = 0;
    vcr_app->video_screen.w = window_width;
    vcr_app->video_screen.h = window_height;

    play_file(&vcr_app->video_player, program->url);

    VcrEvent vcr_event;
    while (video_playing) {
    
        SDL_Event sdl_event;
        if (SDL_WaitEvent(&sdl_event) != 1) {
            printf("SDL event loop error\n");
            // mpv_event *ev = mpv_wait_event(vcr_app->video_player.mpv, 1000);
            // printf("err event: %s\n", mpv_event_name(ev->event_id));
            // if (ev->event_id == MPV_EVENT_LOG_MESSAGE) {
            //     mpv_event_log_message *msg = ev->data;
            //     printf("log: %s", msg->text);
            // }
            return VCR_EVENT_QUIT;  // TODO(cf): Do I need an error event? Maybe?
        }
        
        vcr_event = process_sdl_event(vcr_app, &sdl_event);
        if (vcr_event == VCR_EVENT_QUIT) {
            video_playing = false;
        }

        if (vcr_event == VCR_EVENT_VIDEO_END) {
            video_playing = false;
        }

    }

    printf("fullscreen loop end\n");
    resize_screen(&vcr_app->video_player, vcr_app->renderer, original_screen.w, original_screen.h);
    vcr_app->video_screen.x = original_screen.x;
    vcr_app->video_screen.y = original_screen.y;
    vcr_app->video_screen.w = original_screen.w;
    vcr_app->video_screen.h = original_screen.h;

    return vcr_event;
}

int start_engine(VcrProgrammingQueue *vcr_programming_queue) {

    VcrApplication vcr_app;
    if (init_vcr_application(&vcr_app) != 0) {
        printf("Failed to init vcr display\n");
        destroy_vcr_application(&vcr_app); // TODO(cf): should I call this in init_vcr_app? Maybe?
        return 1;
    }

    const int TARGET_FPS = 24;
    const int FRAME_DELAY = 1000 / TARGET_FPS;

    standby_screen(&vcr_app, default_color_palette(), false);
    render_videoscreen_shadow(&vcr_app, vcr_app.video_screen);
    bool redraw_digital_display = update_digital_clock_and_check_for_redraw(&vcr_app.digital_display_state);
    draw_digital_display(vcr_app.renderer, vcr_app.font_digital_clock_7seg, vcr_app.digital_display, &vcr_app.digital_display_state);

    SDL_Rect logo_rect = {400, 200, 200, 200};
    SDL_Texture *vcr_vector_logo = create_vcr_vector_logo(vcr_app.renderer, 5, PALETTE_DYNAMICRON_BLACK);
    SDL_QueryTexture(vcr_vector_logo, NULL, NULL, &logo_rect.w, &logo_rect.h);
    SDL_SetTextureBlendMode(vcr_vector_logo, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(vcr_app.renderer, vcr_vector_logo, NULL, &logo_rect);
    SDL_SetRenderTarget(vcr_app.renderer, NULL);
    SDL_RenderPresent(vcr_app.renderer);
    SDL_DestroyTexture(vcr_vector_logo);

    bool running = true;
    while (running) {

        int frame_start = SDL_GetTicks();
        SDL_Event sdl_event;
        VcrEvent vcr_event = VCR_EVENT_NONE;

        while (SDL_PollEvent(&sdl_event)) {

            vcr_event = process_sdl_event(&vcr_app, &sdl_event);
            if (vcr_event == VCR_EVENT_QUIT) {
                running = false;
            }

        }

        if (vcr_event == VCR_EVENT_QUIT) {
            break;
        }

        if (!is_empty(vcr_programming_queue)) {
            VcrProgram vcr_program;
            dequeue(vcr_programming_queue, &vcr_program);
            vcr_event = engine_routine_fullscreen_video(&vcr_app, &vcr_program);
        }

        if (vcr_event == VCR_EVENT_QUIT) {
            break;
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
