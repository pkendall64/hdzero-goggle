#ifndef _MODULE_DEF_H
#define _MODULE_DEF_H

typedef struct {
    int num_channels;
    char * (*name)(int channel);
    void (*init)();
    void (*close)();
    void (*set_channel)(int channel);
    void (*set_mode)();
} module_def_t;

char *module_standard_channel_name(int channel);
#endif