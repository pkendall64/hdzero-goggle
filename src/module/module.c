#include "steadyview.h"

static int current_channel = -1;

void module_init() {
    steadyview_init();
}

void module_set_channel(int channel) {
    current_channel = channel;
    if (channel == -1)
        return;

    // use configured module type
    steadyview_set_channel(channel);
}

int module_get_channel() {
    return current_channel;
}
