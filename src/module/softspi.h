#ifndef __MODULE_SOFTSPI_H_
#define __MODULE_SOFTSPI_H_

#include <sys/types.h>

typedef enum {
    SOFTSPI_CLK,
    SOFTSPI_DAT,
    SOFTSPI_CS,
} softspi_pin_t;

typedef enum {
    SOFTSPI_OUTPUT,
    SOFTSPI_INPUT
} softspi_direction_t;

void softspi_init();
void softspi_close();
void softspi_set_direction(softspi_direction_t direction);
void softspi_set_pin(softspi_pin_t pin, int value);
int softspi_get_pin(softspi_pin_t pin);

#endif