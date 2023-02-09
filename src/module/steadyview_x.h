#ifndef __MODULE_STEADYVIEW_X_H_
#define __MODULE_STEADYVIEW_X_H_

#include "module_def.h"

typedef enum {
    STEADYVIEW_X_MIX_1 = 0x81,
    STEADYVIEW_X_DIVERSITY = 0x82,
    STEADYVIEW_X_MIX_2 = 0x83,
    STEADYVIEW_X_MIX_3 = 0x84,
} steadyview_x_mix_mode_t;

extern module_def_t steadyview_x_module;

#endif