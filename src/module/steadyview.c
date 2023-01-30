#include "common.hh"

#include <unistd.h>

#include "steadyview.h"
#include "softspi.h"

#define RX5808_READ_CTRL_BIT      0x00
#define RX5808_WRITE_CTRL_BIT     0x01
#define RX5808_ADDRESS_R_W_LENGTH 5
#define RX5808_DATA_LENGTH        20
#define RX5808_PACKET_LENGTH      25

#define SYNTHESIZER_REG_A 0x00
#define SYNTHESIZER_REG_B 0x01

static const uint16_t frequencyTable[48] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613  // L
};

static int current_channel = -1;
static mix_mode_t mix_mode = -1;

static const uint32_t periodMicroSec = 100;

static void rtc6705WriteRegister(uint32_t data);
static uint32_t rtc6705ReadRegister(uint8_t read);

void steadyview_init() {
    softspi_init();
    softspi_set_pin(SOFTSPI_CLK, 0);
    softspi_set_pin(SOFTSPI_CS, 1);
    softspi_set_pin(SOFTSPI_DAT, 1);
    usleep(100000);

    steadyview_set_channel(32);
    usleep(100000);
    steadyview_set_mixmode(ModeDiversity);
}

void steadyview_close() {
    softspi_close();
}

void steadyview_set_channel(int index) {
    uint16_t f = frequencyTable[index];
    uint32_t data = ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);
    uint32_t newRegisterData = SYNTHESIZER_REG_B | (RX5808_WRITE_CTRL_BIT << 4) | (data << 5);

    uint32_t currentRegisterData = SYNTHESIZER_REG_B | (RX5808_WRITE_CTRL_BIT << 4) | rtc6705ReadRegister(SYNTHESIZER_REG_B);

    if (newRegisterData != currentRegisterData) {
        rtc6705WriteRegister(SYNTHESIZER_REG_A | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
        rtc6705WriteRegister(newRegisterData);
    }
    current_channel = index;
}

void steadyview_set_mixmode(mix_mode_t mode) {
    mix_mode = mode;

    softspi_set_pin(SOFTSPI_CLK, mode & 0x01);
    softspi_set_pin(SOFTSPI_DAT, mode & 0x02);
    usleep(100000);
    softspi_set_pin(SOFTSPI_CLK, 0);
    softspi_set_pin(SOFTSPI_DAT, 1);
    usleep(500000);

    uint16_t f = frequencyTable[current_channel];
    uint32_t data = ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);
    uint32_t registerData = SYNTHESIZER_REG_B | (RX5808_WRITE_CTRL_BIT << 4) | (data << 5);

    rtc6705WriteRegister(SYNTHESIZER_REG_A | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
    usleep(500);
    rtc6705WriteRegister(SYNTHESIZER_REG_A | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
    rtc6705WriteRegister(registerData);
}

static void rtc6705WriteRegister(uint32_t data) {
    softspi_set_pin(SOFTSPI_CS, 0);
    usleep(periodMicroSec);

    for (int i = 0; i < RX5808_PACKET_LENGTH; i++) {
        softspi_set_pin(SOFTSPI_DAT, data & 0x01);
        softspi_set_pin(SOFTSPI_CLK, 1);
        softspi_set_pin(SOFTSPI_CLK, 0);
        data >>= 1;
    }
    softspi_set_pin(SOFTSPI_CS, 1);
    softspi_set_pin(SOFTSPI_DAT, 1);
    usleep(periodMicroSec);
}

static uint32_t rtc6705ReadRegister(uint8_t read) {
    uint32_t buf = read | (RX5808_READ_CTRL_BIT << 4);
    uint32_t registerData = 0;

    softspi_set_pin(SOFTSPI_CS, 0);
    usleep(periodMicroSec);

    // Write register address and read bit
    for (uint8_t i = 0; i < RX5808_ADDRESS_R_W_LENGTH; ++i) {
        softspi_set_pin(SOFTSPI_DAT, buf & 0x01);
        softspi_set_pin(SOFTSPI_CLK, 1);
        softspi_set_pin(SOFTSPI_CLK, 0);
        buf >>= 1;
    }

    // Change pin from output to input
    softspi_set_direction(SOFTSPI_INPUT);

    // Read data 20 bits
    for (uint8_t i = 0; i < RX5808_DATA_LENGTH; i++) {
        softspi_set_pin(SOFTSPI_CLK, 1);
        if (softspi_get_pin(SOFTSPI_DAT)) {
            registerData = registerData | (1 << (5 + i));
        }
        softspi_set_pin(SOFTSPI_CLK, 0);
    }

    // Change pin back to output
    softspi_set_direction(SOFTSPI_OUTPUT);

    softspi_set_pin(SOFTSPI_CS, 1);
    softspi_set_pin(SOFTSPI_DAT, 1);

    return registerData;
}
