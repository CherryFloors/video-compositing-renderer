#include "display.c"
#include "vcr_assets.h"
// #include "spacetime.c"

// int main(void) { run_display_loop(); }
// int main(void) { spacetime(); }
int main(void) {

    VcrApplication app;
    int a = init_vcr_application(&app);

    for (int i = 0; i < 10000; i++) {
        DigitalDisplay *digital_display = create_digital_display(app.renderer);
        destroy_digital_display(digital_display);
        a += i;
    }

    destroy_vcr_application(&app);
}
