// Build with: gcc -o main main.c `pkg-config --libs --cflags mpv sdl2` -std=c99
//
//  // TODO: Use cycle volume for fade out?
//  //https://mpv.io/manual/master/#command-interface-cycle-%3Cname%3E-[%3Cvalue%3E]
//  // const char *cmd_pause[] = {"cycle", "pause", NULL};
//      // vflip filter
//  // const char *cmd_pause[] = {"vf", "set", "vflip", NULL};
//      // Oscope
//  // const char *cmd_pause[] = {"vf", "set", "oscilloscope=x=0.5:y=5/1080:s=1", NULL};
//      //https://video.stackexchange.com/questions/17378/ffmpeg-audio-to-visualisation
//  const char *cmd_pause[] = {"vf", "set", "pixelize=w=200:h=200", NULL};
//  
//  // STOP
//  // const char *cmd_stop[] = {"stop", NULL};
//  mpv_command_async(video_player->mpv, 0, cmd_pause);

#include <stddef.h>
#include <stdio.h>
#include <wchar.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL.h>
#include <mpv/render.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>


// TODO: Rename these using VCR_EVENT... or something
// the or something could be making these a part of the player context for muliple
// players.
static Uint32 wakeup_on_mpv_render_update, wakeup_on_mpv_events;

// This needs destructor...
typedef struct {
    mpv_handle *mpv;
    mpv_render_context *mpv_render;
    SDL_Texture *screen;
    int screen_width;
    int screen_height;
    Uint32 event_wakeup_on_mpv_render_update;
    Uint32 event_wakeup_on_mpv_events;
} VideoPlayerContext;


int init_video_player(VideoPlayerContext *video_player_context, SDL_Renderer *renderer, int w, int h);
int destroy_video_player(VideoPlayerContext *video_player);
int render_video_frame(VideoPlayerContext *video_player, SDL_Renderer *renderer, SDL_Rect *screen_location);
int play_file(VideoPlayerContext *video_player, char *file);

#ifdef VIDEO_PLAYER_IMPLEMENTATION

static void on_mpv_events(void *ctx)
{
    SDL_Event event = {.type = wakeup_on_mpv_events};
    SDL_PushEvent(&event);
}

static void on_mpv_render_update(void *ctx)
{
    SDL_Event event = {.type = wakeup_on_mpv_render_update};
    SDL_PushEvent(&event);
}

int init_video_player(VideoPlayerContext *video_player_context, SDL_Renderer *renderer, int w, int h) {

    video_player_context->mpv = mpv_create();

    if (!video_player_context->mpv) {
        printf("MPV context init failed!\n");
        return 1;
    }

    mpv_set_option_string(video_player_context->mpv, "vo", "libmpv");

    // Some minor options can only be set before mpv_initialize().
    if (mpv_initialize(video_player_context->mpv) < 0) {
        printf("mpv init failed");
        return 1;
    }

    mpv_request_log_messages(video_player_context->mpv, "debug");

    mpv_render_param render_params[] = {
        {MPV_RENDER_PARAM_API_TYPE, MPV_RENDER_API_TYPE_SW},
        // Tell libmpv that you will call mpv_render_context_update() on render
        // context update callbacks, and that you will _not_ block on the core
        // ever (see <libmpv/render.h> "Threading" section for what libmpv
        // functions you can call at all when this is active).
        // In particular, this means you must call e.g. mpv_command_async()
        // instead of mpv_command().
        // If you want to use synchronous calls, either make them on a separate
        // thread, or remove the option below (this will disable features like
        // DR and is not recommended anyway).
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &(int){1}},
        {0}
    };

    int init_render_ctx = mpv_render_context_create(
        &video_player_context->mpv_render,
        video_player_context->mpv,
        render_params
    );

    if (init_render_ctx < 0) {
        printf("failed to initialize mpv GL context\n");
        return 1;
    }

    // We use events for thread-safe notification of the SDL main loop.
    // Generally, the wakeup callbacks (set further below) should do as least
    // work as possible, and merely wake up another thread to do actual work.
    // On SDL, waking up the mainloop is the ideal course of action. SDL's
    // SDL_PushEvent() is thread-safe, so we use that.
    wakeup_on_mpv_render_update = SDL_RegisterEvents(1);
    wakeup_on_mpv_events = SDL_RegisterEvents(1);
    if (wakeup_on_mpv_render_update == (Uint32)-1 || wakeup_on_mpv_events == (Uint32)-1) {
        printf("could not register events\n");
        return 1;
    }

    video_player_context->event_wakeup_on_mpv_render_update = wakeup_on_mpv_render_update;
    video_player_context->event_wakeup_on_mpv_events = wakeup_on_mpv_events;

    // TODO: play around with setting the render update cb parameter to the registered event.
    //
    // When normal mpv events are available.
    mpv_set_wakeup_callback(video_player_context->mpv, on_mpv_events, NULL);

    // When there is a need to call mpv_render_context_update(), which can
    // request a new frame to be rendered.
    // (Separate from the normal event handling mechanism for the sake of
    //  users which run OpenGL on a different thread.)
    mpv_render_context_set_update_callback(video_player_context->mpv_render, on_mpv_render_update, NULL);

    video_player_context->screen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBX8888,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );
    if (!video_player_context->screen) {
        printf("could not allocate texture\n");
        exit(1);
    }

    video_player_context->screen_width = w;
    video_player_context->screen_height = h;

    return 0;
};

int destroy_video_player(VideoPlayerContext *video_player) {

    SDL_DestroyTexture(video_player->screen);
    mpv_render_context_free(video_player->mpv_render);
    mpv_destroy(video_player->mpv);

    return 0;
}

int render_video_frame(VideoPlayerContext *video_player, SDL_Renderer *renderer, SDL_Rect *screen_location) {

    int w, h;
    void *pixels;
    int pitch;

    SDL_QueryTexture(video_player->screen, NULL, NULL, &w, &h);
    if (SDL_LockTexture(video_player->screen, NULL, &pixels, &pitch)) {
        printf("could not lock texture\n");
        exit(1);
    }
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_SW_SIZE, (int[2]){w, h}},
        {MPV_RENDER_PARAM_SW_FORMAT, "0bgr"},
        {MPV_RENDER_PARAM_SW_STRIDE, &(size_t){pitch}},
        {MPV_RENDER_PARAM_SW_POINTER, pixels},
        {0}
    };
    int r = mpv_render_context_render(video_player->mpv_render, params);
    if (r < 0) {
        printf("mpv_render_context_render error: %s\n", mpv_error_string(r));
        exit(1);
    }
    SDL_UnlockTexture(video_player->screen);
    SDL_RenderCopy(renderer, video_player->screen, NULL, screen_location);
    SDL_RenderPresent(renderer);

    return r;
};

int play_file(VideoPlayerContext *video_player, char *file) {
    const char *cmd[] = {"loadfile", file, NULL};
    return mpv_command_async(video_player->mpv, 0, cmd);
}
#endif 
