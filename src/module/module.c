#include "steadyview.h"

static int current_channel = -1;
static char *channels[] = {
    "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8",
    "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8",
    "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
    "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8",
    "L1", "L2", "L3", "L4", "L5", "L6", "L7", "L8",
};

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

int module_num_channels() {
    return 48;
}

char *module_channel_str(int channel) {
    return channels[channel];
}
