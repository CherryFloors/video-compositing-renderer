#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>

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
    SDL_Rect pause;
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
SDL_Texture *create_digital_display_symbol(SDL_Renderer *renderer, TTF_Font *font, const char *text, bool active);
DigitalDisplay *create_digital_display(SDL_Renderer *renderer, TTF_Font *clock_font, TTF_Font *symbol_font);
void destroy_digital_display(DigitalDisplay *digital_display);
void draw_digital_display(SDL_Renderer *renderer, TTF_Font *font, DigitalDisplay *digital_display, DigitalDisplayState *digital_display_state);

#ifdef VCR_ASSETS_IMPLEMENTATION

typedef struct DigitalDisplayTextures {
    SDL_Texture *clock;
    SDL_Texture *channel;
    SDL_Texture *pm;
    SDL_Texture *am;
    SDL_Texture *vcr;
    SDL_Texture *rec;
    SDL_Texture *hifi;
    SDL_Texture *play_arrow_glyph;
    SDL_Texture *pause_bar_glyph;
} DigitalDisplayTextures;

void render_copy_relative(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *relative_to) {
    SDL_Rect dstrect = {srcrect->x + relative_to->x, srcrect->y + relative_to->y, srcrect->w, srcrect->h};
    SDL_RenderCopy(renderer, texture, srcrect, &dstrect);
}

void draw_digital_display(SDL_Renderer *renderer, TTF_Font *font, DigitalDisplay *digital_display, DigitalDisplayState *digital_display_state) {

    SDL_Color active_base_color     = PALETTE_LCD_BLUE_DIM;
    SDL_Color active_bright_color   = PALETTE_LCD_BLUE_BRIGHT;

    SDL_RenderCopy(renderer, digital_display->inactive, NULL, &digital_display->container);

    char fmt_clock[6];
    char fmt_channel[3];
    SDL_snprintf(fmt_clock, sizeof(fmt_clock), "%02d:%02d", digital_display_state->hour, digital_display_state->minute);
    SDL_snprintf(fmt_channel, sizeof(fmt_channel), "%02d", digital_display_state->channel);
    if (digital_display_state->hour < 10) {
        fmt_clock[0] = *"!";
    }

    SDL_Texture *texture_active_clock   = create_glow_text_two_layer(renderer, font, fmt_clock, active_base_color, active_bright_color, true, false);
    SDL_Texture *texture_active_channel = create_glow_text_two_layer(renderer, font, fmt_channel, active_base_color, active_bright_color, true, false);

    SDL_Rect clock_container   = {digital_display->container.x, digital_display->container.y, digital_display->clock.w, digital_display->clock.h};
    SDL_Rect channel_container = {digital_display->container.x + digital_display->channel.x, digital_display->container.y + digital_display->channel.y, digital_display->channel.w, digital_display->channel.h};

    SDL_RenderCopy(renderer, texture_active_clock, NULL, &clock_container);
    SDL_RenderCopy(renderer, texture_active_channel, NULL, &channel_container);

    SDL_DestroyTexture(texture_active_clock);
    SDL_DestroyTexture(texture_active_channel);

    if (digital_display_state->am) {
        render_copy_relative(renderer, digital_display->active, &digital_display->am, &digital_display->container);
    }

    if (digital_display_state->pm) {
        render_copy_relative(renderer, digital_display->active, &digital_display->pm, &digital_display->container);
    }

    if (digital_display_state->vcr) {
        render_copy_relative(renderer, digital_display->active, &digital_display->vcr, &digital_display->container);
    }

    if (digital_display_state->hifi) {
        render_copy_relative(renderer, digital_display->active, &digital_display->hifi, &digital_display->container);
    }

    if (digital_display_state->rec) {
        render_copy_relative(renderer, digital_display->active, &digital_display->rec, &digital_display->container);
    }

    if (digital_display_state->pause) {
        render_copy_relative(renderer, digital_display->active, &digital_display->pause, &digital_display->container);
    }

    if (digital_display_state->play) {
        render_copy_relative(renderer, digital_display->active, &digital_display->play, &digital_display->container);
    }

    if (digital_display_state->fast_forward) {
        render_copy_relative(renderer, digital_display->active, &digital_display->ff, &digital_display->container);
    }

    if (digital_display_state->rewind) {
        render_copy_relative(renderer, digital_display->active, &digital_display->rew, &digital_display->container);
    }

    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // SDL_RenderDrawRect(renderer, &digital_display->pause);
    // SDL_RenderDrawRect(renderer, &digital_display->channel);

}

DigitalDisplayTextures *create_digital_display_textures(SDL_Renderer *renderer, TTF_Font *clock_font, TTF_Font *symbol_font, bool active) {

    DigitalDisplayTextures *digital_display_textures = malloc(sizeof(DigitalDisplay));

    bool digital_font_alpha = false;
    SDL_Color base_color = PALETTE_LCD_INACTIVE_DIM;
    SDL_Color bright_color = PALETTE_LCD_INACTIVE_BRIGHT;
    if (active) {
        digital_font_alpha = true;
        base_color = PALETTE_LCD_BLUE_DIM;
        bright_color = PALETTE_LCD_BLUE_BRIGHT;
    }

    digital_display_textures->clock = create_glow_text_two_layer(renderer, clock_font, "88:88", base_color, bright_color, digital_font_alpha, false);
    digital_display_textures->channel = create_glow_text_two_layer(renderer, clock_font, "88", base_color, bright_color, digital_font_alpha, false);
    digital_display_textures->pm = create_digital_display_symbol(renderer, symbol_font, " PM ", active);
    digital_display_textures->am = create_digital_display_symbol(renderer, symbol_font, " AM ", active);
    digital_display_textures->vcr = create_digital_display_symbol(renderer, symbol_font, "VCR", active);
    digital_display_textures->rec = create_digital_display_symbol(renderer, symbol_font, "REC", active);
    digital_display_textures->hifi = create_digital_display_symbol(renderer, symbol_font, "Hi-Fi", active);

    int clock_height;
    SDL_QueryTexture(digital_display_textures->clock, NULL, NULL, NULL, &clock_height);
    int glyph_height = clock_height / 2;
    int play_glyph_width = (int)(glyph_height * 0.75);
    int pause_bar_glyph_width = glyph_height  / 2;

    digital_display_textures->play_arrow_glyph = create_play_arrow_glyph_texture(renderer, play_glyph_width, glyph_height, active);
    digital_display_textures->pause_bar_glyph = create_pause_bar_glyph(renderer, pause_bar_glyph_width, glyph_height, active);

    return digital_display_textures;
}

void destroy_digital_display_textures(DigitalDisplayTextures *digital_display_textures) {
    SDL_DestroyTexture(digital_display_textures->clock);
    SDL_DestroyTexture(digital_display_textures->channel);
    SDL_DestroyTexture(digital_display_textures->pm);
    SDL_DestroyTexture(digital_display_textures->am);
    SDL_DestroyTexture(digital_display_textures->vcr);
    SDL_DestroyTexture(digital_display_textures->rec);
    SDL_DestroyTexture(digital_display_textures->hifi);
    SDL_DestroyTexture(digital_display_textures->play_arrow_glyph);
    SDL_DestroyTexture(digital_display_textures->pause_bar_glyph);
    free(digital_display_textures);
}

DigitalDisplay *create_digital_display(SDL_Renderer *renderer, TTF_Font *clock_font, TTF_Font *symbol_font) {

    DigitalDisplay *digital_display = malloc(sizeof(DigitalDisplay));
    DigitalDisplayTextures *active_textures = create_digital_display_textures(renderer, clock_font, symbol_font, true);
    DigitalDisplayTextures *inactive_textures = create_digital_display_textures(renderer, clock_font, symbol_font, false);

    SDL_QueryTexture(inactive_textures->clock, NULL, NULL, &digital_display->clock.w, &digital_display->clock.h);
    SDL_QueryTexture(inactive_textures->channel, NULL, NULL, &digital_display->channel.w, &digital_display->channel.h);
    SDL_QueryTexture(inactive_textures->pm, NULL, NULL, &digital_display->pm.w, &digital_display->pm.h);
    SDL_QueryTexture(inactive_textures->am, NULL, NULL, &digital_display->am.w, &digital_display->am.h);
    SDL_QueryTexture(inactive_textures->vcr, NULL, NULL, &digital_display->vcr.w, &digital_display->vcr.h);
    SDL_QueryTexture(inactive_textures->rec, NULL, NULL, &digital_display->rec.w, &digital_display->rec.h);
    SDL_QueryTexture(inactive_textures->hifi, NULL, NULL, &digital_display->hifi.w, &digital_display->hifi.h);

    int glyph_height, play_glyph_width, pause_bar_glyph_width;
    SDL_QueryTexture(inactive_textures->play_arrow_glyph, NULL, NULL, &play_glyph_width, &glyph_height);
    SDL_QueryTexture(inactive_textures->pause_bar_glyph, NULL, NULL, &pause_bar_glyph_width, NULL);

    int glyph_spacing                   = pause_bar_glyph_width;
    int space_between_clock_and_channel = glyph_height;
    int top_row_width                   = digital_display->clock.w + digital_display->am.w + digital_display->channel.w + space_between_clock_and_channel;
    int bottom_row_width                = digital_display->vcr.w + digital_display->rec.w + digital_display->hifi.w + (play_glyph_width * 4) + (pause_bar_glyph_width * 2) + (glyph_spacing * 5);
    int bottom_symbol_space             = (top_row_width - bottom_row_width) / 3;
    int bottom_row_y                    = digital_display->clock.h + 10;

    SDL_Rect container_rew1   = { 0, bottom_row_y,      play_glyph_width, glyph_height };
    SDL_Rect container_rew2   = { 0, bottom_row_y,      play_glyph_width, glyph_height };
    SDL_Rect container_pause1 = { 0, bottom_row_y, pause_bar_glyph_width, glyph_height };
    SDL_Rect container_pause2 = { 0, bottom_row_y, pause_bar_glyph_width, glyph_height };
    SDL_Rect container_play   = { 0, bottom_row_y,      play_glyph_width, glyph_height };
    SDL_Rect container_ff     = { 0, bottom_row_y,      play_glyph_width, glyph_height };

    digital_display->clock.x   = 0;
    digital_display->clock.y   = 0;
    digital_display->am.x      = digital_display->clock.w;
    digital_display->am.y      = 0;
    digital_display->pm.x      = digital_display->clock.w;
    digital_display->pm.y      = digital_display->clock.h - digital_display->pm.h;
    digital_display->channel.x = digital_display->clock.w + digital_display->am.w + space_between_clock_and_channel;
    digital_display->channel.y = 0;

    digital_display->vcr.x     = 0;
    digital_display->vcr.y     = bottom_row_y;
    container_rew1.x           = digital_display->vcr.w + bottom_symbol_space;
    container_rew2.x           = container_rew1.x + play_glyph_width + glyph_spacing;
    container_pause1.x         = container_rew2.x + play_glyph_width + glyph_spacing;
    container_pause2.x         = container_pause1.x + pause_bar_glyph_width + glyph_spacing;
    container_play.x           = container_pause2.x + pause_bar_glyph_width + glyph_spacing;
    container_ff.x             = container_play.x + play_glyph_width + glyph_spacing;
    digital_display->rec.x     = container_ff.x + play_glyph_width + bottom_symbol_space;
    digital_display->rec.y     = bottom_row_y;
    digital_display->hifi.x    = digital_display->rec.x + digital_display->rec.w + bottom_symbol_space;
    digital_display->hifi.y    = bottom_row_y;

    digital_display->container.x = 0;
    digital_display->container.y = 0;
    digital_display->container.w = top_row_width;
    digital_display->container.h = bottom_row_y + digital_display->rec.h;

    digital_display->rew.x = container_rew1.x;
    digital_display->rew.y = container_rew1.y;
    digital_display->rew.w = (container_rew2.x - container_rew1.x) + container_rew2.w;
    digital_display->rew.h = container_rew1.h;

    digital_display->pause.x = container_pause1.x;
    digital_display->pause.y = container_pause1.y;
    digital_display->pause.w = (container_pause2.x - container_pause1.x) + container_pause2.w;
    digital_display->pause.h = container_pause1.h;

    digital_display->play.x = container_play.x;
    digital_display->play.y = container_play.y;
    digital_display->play.w = container_play.w;
    digital_display->play.h = container_play.h;

    digital_display->ff.x = container_play.x;
    digital_display->ff.y = container_play.y;
    digital_display->ff.w = (container_ff.x - container_play.x) + container_ff.w;
    digital_display->ff.h = container_play.h;

    digital_display->active = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, digital_display->container.w, digital_display->container.h);
    digital_display->inactive = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, digital_display->container.w, digital_display->container.h);

    // Draw active textures
    SDL_SetRenderTarget(renderer, digital_display->active);
    SDL_SetTextureBlendMode(digital_display->active, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, PALETTE_LCD_BACKGROUND.r, PALETTE_LCD_BACKGROUND.g, PALETTE_LCD_BACKGROUND.b, PALETTE_LCD_BACKGROUND.a);
    SDL_RenderFillRect(renderer, NULL);

    SDL_RenderCopy(renderer, active_textures->clock, NULL, &digital_display->clock);
    SDL_RenderCopy(renderer, active_textures->am, NULL, &digital_display->am);
    SDL_RenderCopy(renderer, active_textures->pm, NULL, &digital_display->pm);
    SDL_RenderCopy(renderer, active_textures->channel, NULL, &digital_display->channel);
    SDL_RenderCopy(renderer, active_textures->vcr, NULL, &digital_display->vcr);
    SDL_RenderCopy(renderer, active_textures->rec, NULL, &digital_display->rec);
    SDL_RenderCopy(renderer, active_textures->hifi, NULL, &digital_display->hifi);
    SDL_RenderCopyEx(renderer, active_textures->play_arrow_glyph, NULL, &container_rew1, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopyEx(renderer, active_textures->play_arrow_glyph, NULL, &container_rew2, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopy(renderer, active_textures->pause_bar_glyph, NULL, &container_pause1);
    SDL_RenderCopy(renderer, active_textures->pause_bar_glyph, NULL, &container_pause2);
    SDL_RenderCopy(renderer, active_textures->play_arrow_glyph, NULL, &container_play);
    SDL_RenderCopy(renderer, active_textures->play_arrow_glyph, NULL, &container_ff);

    // Draw inactive textures
    SDL_SetRenderTarget(renderer, digital_display->inactive);
    SDL_SetTextureBlendMode(digital_display->inactive, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, PALETTE_LCD_BACKGROUND.r, PALETTE_LCD_BACKGROUND.g, PALETTE_LCD_BACKGROUND.b, PALETTE_LCD_BACKGROUND.a);
    SDL_RenderFillRect(renderer, NULL);

    SDL_RenderCopy(renderer, inactive_textures->clock, NULL, &digital_display->clock);
    SDL_RenderCopy(renderer, inactive_textures->am, NULL, &digital_display->am);
    SDL_RenderCopy(renderer, inactive_textures->pm, NULL, &digital_display->pm);
    SDL_RenderCopy(renderer, inactive_textures->channel, NULL, &digital_display->channel);
    SDL_RenderCopy(renderer, inactive_textures->vcr, NULL, &digital_display->vcr);
    SDL_RenderCopy(renderer, inactive_textures->rec, NULL, &digital_display->rec);
    SDL_RenderCopy(renderer, inactive_textures->hifi, NULL, &digital_display->hifi);
    SDL_RenderCopyEx(renderer, inactive_textures->play_arrow_glyph, NULL, &container_rew1, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopyEx(renderer, inactive_textures->play_arrow_glyph, NULL, &container_rew2, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopy(renderer, inactive_textures->pause_bar_glyph, NULL, &container_pause1);
    SDL_RenderCopy(renderer, inactive_textures->pause_bar_glyph, NULL, &container_pause2);
    SDL_RenderCopy(renderer, inactive_textures->play_arrow_glyph, NULL, &container_play);
    SDL_RenderCopy(renderer, inactive_textures->play_arrow_glyph, NULL, &container_ff);

    SDL_SetRenderTarget(renderer, NULL);

    destroy_digital_display_textures(active_textures);
    destroy_digital_display_textures(inactive_textures);

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

SDL_Texture *create_digital_display_symbol(SDL_Renderer *renderer, TTF_Font *font, const char *text, bool active) {

    SDL_Color active_base_color     = PALETTE_LCD_BLUE_DIM;
    SDL_Color active_bright_color   = PALETTE_LCD_BLUE_BRIGHT;
    SDL_Color inactive_base_color   = PALETTE_LCD_INACTIVE_DIM;
    SDL_Color inactive_bright_color = PALETTE_LCD_INACTIVE_BRIGHT;

    if (active) {
        return create_glow_text_two_layer(renderer, font, text, active_base_color, active_bright_color, true, true);
    } else {
        return create_glow_text_two_layer(renderer, font, text, inactive_base_color, inactive_bright_color, true, false);
    }
}
#endif 
