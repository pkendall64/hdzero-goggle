#ifndef __MODULE_STEADYVIEW_H_
#define __MODULE_STEADYVIEW_H_

#include "module_def.h"

typedef enum {
    STEADYVIEW_DIVERSITY,
    STEADYVIEW_MIX_1,
    STEADYVIEW_MIX_2,
    STEADYVIEW_MIX_3,
} steadyview_mix_mode_t;

extern module_def_t steadyview_module;

#endif
