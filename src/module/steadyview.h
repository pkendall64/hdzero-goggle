#ifndef __MODULE_STEADYVIEW_H_
#define __MODULE_STEADYVIEW_H_

typedef enum {
    ModeDiversity,
    ModeMix1,
    ModeMix2,
    ModeMix3,
} mix_mode_t;

void steadyview_init();
void steadyview_close();
void steadyview_set_channel(int index);
void steadyview_set_mixmode(mix_mode_t mode);

#endif
