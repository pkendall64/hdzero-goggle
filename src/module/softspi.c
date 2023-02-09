#include "common.hh"

#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "driver/gpio.h"

#include "softspi.h"

static int pins_fd[3] = {-1, -1, -1};
static int dat_dir_fd = -1;

static int open_pin(int port_num) {
    char buf[64];
    sprintf(buf, "/sys/class/gpio/gpio%d/value", port_num);
    return open(buf, O_RDWR);
}

void softspi_init() {
    gpio_open(GPIO_MODULE_CLK);
    pins_fd[SOFTSPI_CLK] = open_pin(GPIO_MODULE_CLK);

    gpio_open(GPIO_MODULE_CS);
    pins_fd[SOFTSPI_CS] = open_pin(GPIO_MODULE_CS);

    gpio_open(GPIO_MODULE_DAT);
    pins_fd[SOFTSPI_DAT] = open_pin(GPIO_MODULE_DAT);

    char buf[64];
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", GPIO_MODULE_DAT);
    dat_dir_fd = open(buf, O_WRONLY);
}

void softspi_release_gpio(int port_num)
{
	char buf[64];
    sprintf(buf, "echo \"%d\">/sys/class/gpio/unexport", port_num);
    system(buf);
}

void softspi_close() {
    if (pins_fd[SOFTSPI_CLK] != -1) {
        close(pins_fd[SOFTSPI_CLK]);
        pins_fd[SOFTSPI_CLK] = -1;
        softspi_release_gpio(GPIO_MODULE_CLK);
    }
    if (pins_fd[SOFTSPI_DAT] != -1) {
        close(pins_fd[SOFTSPI_DAT]);
        pins_fd[SOFTSPI_DAT] = -1;
        softspi_release_gpio(GPIO_MODULE_DAT);
    }
    if (pins_fd[SOFTSPI_CS] != -1) {
        close(pins_fd[SOFTSPI_CS]);
        pins_fd[SOFTSPI_CS] = -1;
        softspi_release_gpio(GPIO_MODULE_CS);
    }
}

void softspi_set_pin(softspi_pin_t pin, int value) {
    write(pins_fd[pin], value ? "1" : "0", 1);
}

int softspi_get_pin(softspi_pin_t pin) {
    uint8_t buf[1];
    lseek(pins_fd[pin], SEEK_SET, 0);
    read(pins_fd[pin], buf, 1);
    return buf[0] == '1';
}

void softspi_set_direction(softspi_direction_t direction) {
    lseek(dat_dir_fd, SEEK_SET, 0);
    if (direction == SOFTSPI_OUTPUT)
        write(dat_dir_fd, "out\n", 4);
    else
        write(dat_dir_fd, "in\n", 3);
}
