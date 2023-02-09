#include <stdlib.h>
#include <unistd.h>

#include "core/common.hh"
#include "core/settings.h"

#include "driver/gpio.h"
#include "driver/uart.h"

#include "softspi.h"
#include "steadyview_x.h"

static int uart_fd = -1;

static void send_command(uint8_t *bytes, uint8_t length) {
    uint8_t crc = 0;
    for (int i=1 ; i<length-2 ; i++) {
        crc ^= bytes[i];
    }
    bytes[length-2] = crc;

    // we try a few times becasue the software uart is not super accurate at 115200 baud!
    for (int i=0; i<10 ; i++)
        uart_write(uart_fd, bytes, length);
}

static void steadyview_x_set_channel(int index) {
    uint8_t data[] = {0x02, 0x06, 0x31, index, 0x00, 0x03};
    send_command(data, sizeof(data));
}

void steadyview_x_set_mixmode(steadyview_x_mix_mode_t mode) {
    uint8_t data[] = {0x02, 0x06, 0x34, (uint8_t)mode, 0x00, 0x03};
    send_command(data, sizeof(data));
}

static void steadyview_x_init() {
    gpio_open(GPIO_MODULE_CS);
    gpio_set(GPIO_MODULE_CS, 0);

    system("insmod /mnt/app/ko/soft_uart.ko");

    usleep(100000);
    uart_fd = uart_open(4);
    if (uart_fd >= 0) {
        steadyview_x_set_channel(g_setting.module.channel-1);
        usleep(100000);
        steadyview_x_set_mixmode(STEADYVIEW_X_DIVERSITY);
    }
}

static void steadyview_x_close() {
    uart_close(uart_fd);
    usleep(100000);
    system("rmmod soft_uart");
    softspi_release_gpio(GPIO_MODULE_CS);
}

module_def_t steadyview_x_module = {
    48,
    module_standard_channel_name,
    steadyview_x_init,
    steadyview_x_close,
    steadyview_x_set_channel
};