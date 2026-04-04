#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

const SDL_Color PALETTE_SUPERCOLOR_BLUE     = {0x03, 0x8D, 0xAE, 0xFF};
const SDL_Color PALETTE_SUPERCOLOR_GREEN    = {0x8E, 0xA4, 0x3B, 0xFF};
const SDL_Color PALETTE_SUPERCOLOR_YELLOW   = {0xE9, 0xBD, 0x01, 0xFF};
const SDL_Color PALETTE_SUPERCOLOR_ORANGE   = {0xF2, 0x47, 0x01, 0xFF};
const SDL_Color PALETTE_SUPERCOLOR_RED      = {0xF8, 0x01, 0x50, 0xFF};
const SDL_Color PALETTE_SUPERCOLOR_WHITE    = {0xE8, 0xE1, 0xD6, 0xFF};
const SDL_Color PALETTE_SUPERCOLOR_BLACK    = {0x18, 0x18, 0x18, 0xFF};

const SDL_Color PALETTE_DYNAMICRON_YELLOW   = {0xFD, 0xC3, 0x31, 0xFF};
const SDL_Color PALETTE_DYNAMICRON_ORANGE   = {0xFE, 0x57, 0x22, 0xFF};
const SDL_Color PALETTE_DYNAMICRON_RED      = {0xF0, 0x35, 0x3C, 0xFF};
const SDL_Color PALETTE_DYNAMICRON_DARK_RED = {0xB4, 0x21, 0x3D, 0xFF};
const SDL_Color PALETTE_DYNAMICRON_PURPLE   = {0x67, 0x1C, 0x3B, 0xFF};
const SDL_Color PALETTE_DYNAMICRON_WHITE    = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color PALETTE_DYNAMICRON_BLACK    = {0x1A, 0x1A, 0x1A, 0xFF};

const SDL_Color PALETTE_LCD_BACKGROUND      = {0x00, 0x00, 0x00, 0xFF};
const SDL_Color PALETTE_LCD_BLUE_DIM        = {0x1A, 0xFD, 0xD7, 0xFF};
const SDL_Color PALETTE_LCD_BLUE_BRIGHT     = {0xBD, 0xFF, 0xFD, 0xFF};
// Inactive bright and dim are flipped here on purpose
const SDL_Color PALETTE_LCD_INACTIVE_DIM    = {0x3A, 0x3A, 0x3A, 0xFF};
const SDL_Color PALETTE_LCD_INACTIVE_BRIGHT = {0x25, 0x25, 0x25, 0xFF};

const SDL_Color PALETTE_CLEAR               = {0xFF, 0xFF, 0xFF, 0x00};

typedef struct PlayArrowGlyph {
    SDL_Vertex layer0[3];
    SDL_Vertex layer1[3];
    SDL_Vertex layer2[3];
    SDL_Vertex layer3[3];
} PlayArrowGlyph;

typedef struct PlayArrow {
    SDL_Vertex top;
    SDL_Vertex bottom;
    SDL_Vertex point;
} PlayArrow;

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

typedef struct DigitalDisplay {
    SDL_Rect clock;
    SDL_Rect channel;
    SDL_Rect pm;
    SDL_Rect am;
    SDL_Rect vcr;
    SDL_Rect rec;
    SDL_Rect hifi;
    SDL_Rect play;
    SDL_Rect ff;
    SDL_Rect rew;
    SDL_Rect glyph;
    SDL_Rect container;
    SDL_Texture *active;
    SDL_Texture *inactive;
} DigitalDisplay;

typedef struct VcrColorPalette {
    SDL_Color c0;
    SDL_Color c1;
    SDL_Color c2;
    SDL_Color c3;
    SDL_Color c4;
    SDL_Color text_fg;
    SDL_Color text_bg;
} VcrColorPalette;

PlayArrow build_play_arrow(int width, int height);
PlayArrow annular_play_arrow(PlayArrow play_arrow, float delta);
SDL_Texture *create_pause_bar_glyph(SDL_Renderer *renderer, int tex_width, int tex_height, bool active);
SDL_Texture *create_play_arrow_glyph_texture(SDL_Renderer *renderer, int width, int height, bool active);
SDL_Texture *create_glow_text_two_layer(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color base_color, SDL_Color bright_color, bool alpha, bool small_font);
DigitalDisplay *create_digital_display(SDL_Renderer *renderer);
void destroy_digital_display(DigitalDisplay *digital_display);

#ifdef VCR_ASSETS_IMPLEMENTATION

DigitalDisplay *create_digital_display(SDL_Renderer *renderer) {
    DigitalDisplay *digital_display = malloc(sizeof(DigitalDisplay));
    digital_display->active =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 100, 100);
    digital_display->inactive =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 100, 100);
    return digital_display;
}

void destroy_digital_display(DigitalDisplay *digital_display) {
    SDL_DestroyTexture(digital_display->active);
    SDL_DestroyTexture(digital_display->inactive);
    free(digital_display);
};

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

    SDL_Color color_dim = PALETTE_LCD_BLUE_DIM;
    SDL_Color color_bright = PALETTE_LCD_BLUE_BRIGHT;
    color_dim = color_bright; // TODO: small glyphs arent nesting well so one color
    if (!active) {
        color_dim = PALETTE_LCD_INACTIVE_BRIGHT;
        color_bright = PALETTE_LCD_INACTIVE_BRIGHT;
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

PlayArrow build_play_arrow(int width, int height) {

    PlayArrow play_arrow;

    float x                    = (float)width;
    float y                    = (float)height;
    float mid_point            = y / 2.0;

    SDL_FPoint top_position    = { 0.0,        0.0};
    SDL_FPoint bottom_position = { 0.0,         y };
    SDL_FPoint point_position  = {   x, mid_point };

    play_arrow.top.position    = top_position;
    play_arrow.bottom.position = bottom_position;
    play_arrow.point.position  = point_position;

    return play_arrow;
}

PlayArrow annular_play_arrow(PlayArrow play_arrow, float delta) {

    PlayArrow annular_arrow;

    double half_height = play_arrow.point.position.y - play_arrow.top.position.y;
    double arrow_width = play_arrow.point.position.x - play_arrow.top.position.x;

    if (delta >= (arrow_width / 2.0)) {
        delta = arrow_width / 2.0;
    }

    double slope                     = half_height / arrow_width;
    double top_to_point_length       = sqrt(half_height * half_height + arrow_width * arrow_width);
    double tangent_of_top_half_angle = (top_to_point_length - half_height) / arrow_width;
    double delta_y_factor            = delta / tangent_of_top_half_angle;

    float top_x    = play_arrow.top.position.x + delta;
    float top_y    = play_arrow.top.position.y + delta_y_factor;
    float bottom_x = top_x;
    float bottom_y = play_arrow.bottom.position.y - delta_y_factor;
    float point_x  = ((half_height - top_y) / slope) + top_x;
    float point_y  = play_arrow.point.position.y;

    annular_arrow.top.position    = (SDL_FPoint){    top_x,    top_y };
    annular_arrow.bottom.position = (SDL_FPoint){ bottom_x, bottom_y };
    annular_arrow.point.position  = (SDL_FPoint){  point_x,  point_y };

    return annular_arrow;
}

SDL_Texture *create_play_arrow_glyph_texture(SDL_Renderer *renderer, int width, int height, bool active) {

    SDL_Texture *target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);

    SDL_SetRenderTarget(renderer, target);
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);

    SDL_Color color_annulus   = PALETTE_CLEAR;
    SDL_Color color_dim       = PALETTE_LCD_BLUE_DIM;
    SDL_Color color_bright    = PALETTE_LCD_BLUE_BRIGHT;

    color_dim = color_bright; // TODO: small glyphs arent nesting well so one color
    if (!active) {
        color_dim    = PALETTE_LCD_INACTIVE_BRIGHT;
        color_bright = PALETTE_LCD_INACTIVE_BRIGHT;
    }

    PlayArrow base_layer       = build_play_arrow(width, height);
    base_layer.top.color       = color_dim;
    base_layer.bottom.color    = color_dim;
    base_layer.point.color     = color_dim;

    float delta                = (float)width / 4.0;
    PlayArrow annular_layer    = annular_play_arrow(base_layer, delta);
    annular_layer.top.color    = color_annulus;
    annular_layer.bottom.color = color_annulus;
    annular_layer.point.color  = color_annulus;
    
    SDL_RenderGeometry(renderer, NULL, &base_layer, 3, NULL, 0);
    SDL_RenderGeometry(renderer, NULL, &annular_layer, 3, NULL, 0);
    SDL_SetRenderTarget(renderer, NULL);

    return target;
}

SDL_Texture *create_glow_text_two_layer(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color base_color, SDL_Color bright_color, bool alpha, bool small_font) {

    SDL_Color active_text_center_color = bright_color;
    SDL_Color active_text_outline_color = {base_color.r, base_color.g, base_color.b};
    if (alpha) {
        active_text_outline_color.a = 120;
    }

    int border_width = 1;
    TTF_SetFontOutline(font, border_width);

    SDL_Surface *surface_outline = TTF_RenderText_Blended(font, text, active_text_outline_color);
    if (!small_font) {
        TTF_SetFontOutline(font, 0);
    }
    SDL_Surface *surface_center = TTF_RenderText_Blended(font, text, active_text_center_color);
    TTF_SetFontOutline(font, 0);

    SDL_Texture *texture_outline = SDL_CreateTextureFromSurface(renderer, surface_outline);
    SDL_Texture *texture_center = SDL_CreateTextureFromSurface(renderer, surface_center);

    int target_width = surface_outline->w;
    int target_height = surface_outline->h;
    SDL_Rect aligned_center_text = { (target_width - surface_center->w) / 2, (target_height - surface_center->h) / 2, surface_center->w, surface_center->h };
    SDL_Texture *target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, target_width, target_height);
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, target);

    if (small_font) {
        SDL_RenderCopy(renderer, texture_outline, NULL, NULL);
        SDL_RenderCopy(renderer, texture_center, NULL, &aligned_center_text);
    } else {
        SDL_RenderCopy(renderer, texture_center, NULL, &aligned_center_text);
        SDL_RenderCopy(renderer, texture_outline, NULL, NULL);
    }

    SDL_FreeSurface(surface_outline);
    SDL_FreeSurface(surface_center);
    SDL_DestroyTexture(texture_outline);
    SDL_DestroyTexture(texture_center);
    SDL_SetRenderTarget(renderer, NULL);

    return target;
};
#endif 
