#ifndef __MODULE_MODULE_H_
#define __MODULE_MODULE_H_

void module_init();
void module_close();
void module_set_channel(int channel);

int module_num_channels();
char *module_channel_str(int channel);

#endif
