#include "core/settings.h"

#include "steadyview.h"

static module_def_t *module = &steadyview_module;

void module_init() {
    if (g_setting.module.channel > module->num_channels)
        g_setting.module.channel = 1;
    module->init();
}

void module_close() {
    module->close();
}

void module_set_channel(int channel) {
    if (channel == -1)
        return;
    module->set_channel(channel-1);
}

int module_num_channels() {
    return module->num_channels;
}

char *module_channel_str(int channel) {
    return module->name(channel-1);
}
