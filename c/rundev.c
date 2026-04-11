#define VCR_PROGRAMMING_QUEUE_IMPLEMENTATION
#define VCR_ASSETS_IMPLEMENTATION
#define VIDEO_PLAYER_IMPLEMENTATION
#include "vcr_programming_queue.h"
#include "engine.c"
// #include "vcr_assets.h"
// #include "spacetime.c"

int main(void) {
    ProgrammingQueue programming_queue;
    start_engine(&programming_queue);
}
// int main(void) { spacetime(); }
// int main(void) {
//
//     VcrApplication app;
//     int a = init_vcr_application(&app);
//     DigitalDisplayState display_state = {
//         2,
//         0,
//         4,
//         false,
//         true,
//         true,
//         true,
//         false,
//         true,
//         true,
//         false,
//         false,
//     };
//
//     for (int i = 0; i < 100; i++) {
//         DigitalDisplay *digital_display = create_digital_display(app.renderer, app.font_digital_clock_7seg, app.font_default);
//         draw_digital_display(app.renderer, app.font_digital_clock_7seg, digital_display, &display_state);
//         destroy_digital_display(digital_display);
//         a += i;
//     }
//
//     destroy_vcr_application(&app);
// }
