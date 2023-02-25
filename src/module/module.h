#ifndef __MODULE_MODULE_H_
#define __MODULE_MODULE_H_

typedef enum {
    MODULE_UNKNOWN,
    MODULE_SWITCH,
    MODULE_RAPIDFIRE,
    MODULE_STEADYVIEW,
    MODULE_STEADYVIEW_X,
} module_type_t;

void module_init();
void module_close();
void module_set_channel(int channel);

int module_num_channels();
char *module_channel_str(int channel);

#endif
