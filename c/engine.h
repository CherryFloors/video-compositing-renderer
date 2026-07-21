#pragma once
#include "vcr_programming_queue.h"

static const int RES_SD_W = 640;
static const int RES_SD_H = 480;
static const int RES_FHD_W = 1920;
static const int RES_FHD_H = 1080;

typedef enum DisplayResolution {
    RESOLUTION_SD_640_480,
    RESOLUTION_FHD_1920_1080,
} DisplayResolution;

typedef enum VcrEvent {
    VCR_EVENT_NONE      = 0,
    VCR_EVENT_QUIT      = 1,
    VCR_EVENT_VIDEO_END = 2,
} VcrEvent;

int start_engine(VcrProgrammingQueue *program_queue);
