#include "core/common.hh"
#include "core/settings.h"

#include "rapidfire.h"
#include "steadyview.h"
#include "steadyview_x.h"
#include "switch.h"

static module_def_t *modules[] = {
    NULL, // unknown/uncontrolled
    &switch_module,
    &rapidfire_module,
    &steadyview_module,
    &steadyview_x_module
};

static module_def_t *module = NULL;

void module_init() {
    module = modules[g_setting.module.type];
    if (module) {
        if (g_setting.module.channel > module->num_channels)
            g_setting.module.channel = 1;
        module->init();
    }
}

void module_set_mode() {
    module = modules[g_setting.module.type];
    if (module) {
        if (g_setting.module.channel > module->num_channels)
            g_setting.module.channel = 1;
        module->set_mode();
    }
}

void module_close() {
    if (module)
        module->close();
}

void module_set_channel(int channel) {
    if (channel == -1)
        return;
    if (module)
        module->set_channel(channel - 1);
}

int module_num_channels() {
    if (module)
        return module->num_channels;
    return 0;
}

char *module_channel_str(int channel) {
    if (module)
        return module->name(channel - 1);
    return "";
}
