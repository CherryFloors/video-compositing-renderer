#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>


typedef struct PlayArrowGlyph {
    SDL_Vertex layer0[3];
    SDL_Vertex layer1[3];
    SDL_Vertex layer2[3];
    SDL_Vertex layer3[3];
} PlayArrowGlyph;

typedef struct PauseBarGlyph {
    SDL_Rect layer0;
    SDL_Rect layer1;
    SDL_Rect layer2;
    SDL_Rect layer3;
} PauseBarGlyph;

typedef struct DigitalDisplayState {
    int hour;
    int minute;
    int channel;
    bool am;
    bool pm;
    bool vcr;
    bool hifi;
    bool rec;
    bool pause;
    bool play;
    bool fast_forward;
    bool rewind;
} DigitalDisplayState;

typedef struct VcrColorPalette {
    SDL_Color c0;
    SDL_Color c1;
    SDL_Color c2;
    SDL_Color c3;
    SDL_Color c4;
    SDL_Color text_fg;
    SDL_Color text_bg;
} VcrColorPalette;

SDL_Texture *create_pause_bar_glyph(SDL_Renderer *renderer, int tex_width, int tex_height, bool active);

#ifdef VCR_ASSETS_IMPLEMENTATION

SDL_Texture *create_pause_bar_glyph(SDL_Renderer *renderer, int tex_width, int tex_height, bool active) {

    // TODO: Using add blend mode could be done to auto brighten color

    // TODO: Try letting the outline colors be the bright color with an alpha so they appear darker when drawn
    // on the black background

    SDL_Rect glyph_container = { 0, 0, tex_width, tex_height }; 
    SDL_Texture *target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, glyph_container.w, glyph_container.h);

    SDL_SetRenderTarget(renderer, target);
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);

    float x = 0.0;
    float y = 0.0;
    float height = (float)tex_height;
    float glyph_width = (float)tex_width;

    float width = glyph_width * 0.25;
    float outline_width = width * 0.25;

    SDL_Color color_dim = {0x1A, 0xFD, 0xD7, 0xff};
    SDL_Color color_bright = {0xBD, 0xFF, 0xFD, 0xff};
    color_dim = color_bright; // TODO: small glyphs arent nesting well so one color
    if (!active) {
        color_dim = (SDL_Color){0x25, 0x25, 0x25, 0xff};
        color_bright = (SDL_Color){0x25, 0x25, 0x25, 0xff};
    }

    float layer2_delta = width - outline_width;
    SDL_FRect layer1 = {x + outline_width, y + outline_width, glyph_width - (2 * outline_width),
                       height - (2 * outline_width)};
    SDL_FRect layer2 = {x + layer2_delta, y + layer2_delta, glyph_width - (2 * layer2_delta),
                       height - (2 * layer2_delta)};
    SDL_FRect layer3 = {x + width, y + width, glyph_width - (2 * width), height - (2 * width)};

    SDL_SetRenderDrawColor(renderer, color_dim.r, color_dim.g, color_dim.b, 0xff);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, color_bright.r, color_bright.g, color_bright.b, color_bright.a);
    SDL_RenderFillRectF(renderer, &layer1);

    SDL_SetRenderDrawColor(renderer, color_dim.r, color_dim.g, color_dim.b, color_dim.a);
    SDL_RenderFillRectF(renderer, &layer2);

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
    SDL_RenderFillRectF(renderer, &layer3);

    SDL_SetRenderTarget(renderer, NULL);

    return target;
}

#endif 
