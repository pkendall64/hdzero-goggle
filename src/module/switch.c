#include <unistd.h>

#include "core/common.hh"
#include "core/settings.h"

#include "softspi.h"
#include "switch.h"

#define BAY_PIN1 SOFTSPI_CLK
#define BAY_PIN2 SOFTSPI_DAT
#define BAY_PIN3 SOFTSPI_CS

static void switch_set_channel(int channel) {
    softspi_set_pin(BAY_PIN1, (channel % 8) & 1);
    softspi_set_pin(BAY_PIN2, (channel % 8) & 2);
    softspi_set_pin(BAY_PIN3, (channel % 8) & 4);
}

static void switch_init() {
    softspi_init();
    // Set the swicthes/GPIO to a state that is NOT part of the rapidfire SPI setup.
    switch_set_channel(2);
    usleep(200000);
    switch_set_channel(g_setting.module.channel - 1);
}

static void switch_close() {
    softspi_close();
}

static char *switch_name(int channel) {
    static char *channel_name[] = {
        "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8",
    };
    return channel_name[channel];
}

module_def_t switch_module = {
    8,
    switch_name,
    switch_init,
    switch_close,
    switch_set_channel};