#pragma once
#include "vcr_programming_queue.h"
typedef enum DisplayResolution {
    RESOULUTION_SD_640_480,
    RESOULUTION_FHD_1920_1080,
} DisplayResolution;


typedef enum VcrEvent {
    VCR_EVENT_NONE      = 0,
    VCR_EVENT_QUIT      = 1,
    VCR_EVENT_VIDEO_END = 2,
} VcrEvent;

int start_engine(VcrProgrammingQueue *program_queue);
