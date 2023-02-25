#include <unistd.h>

#include "core/common.hh"
#include "core/settings.h"

#include "softspi.h"
#include "rapidfire.h"

#define SPAM_COUNT          3

#define RF_API_DIR_GRTHAN   '>'
#define RF_API_DIR_EQUAL    '='
#define RF_API_BEEP_CMD     'S'
#define RF_API_BAND_CMD     'B'
#define RF_API_CHANNEL_CMD  'C'
#define RF_API_MODE_CMD     'D'

static int current_channel = -1;

static void send_band(uint8_t band);
static void send_channel(uint8_t channel);
static void send_mode(uint8_t mode);

static void rapidfire_set_channel(int index) {
    uint8_t band = index / 8;
    uint8_t channel = index % 8;

    send_band(band);
	usleep(100000);
    send_channel(channel);

    current_channel = index;
}

static void rapidfire_init() {
    softspi_init();

    softspi_set_pin(SOFTSPI_CLK, 1);
    softspi_set_pin(SOFTSPI_CS, 1);
    softspi_set_pin(SOFTSPI_DAT, 1);
    usleep(200000);
    softspi_set_pin(SOFTSPI_CLK, 0);
    softspi_set_pin(SOFTSPI_CS, 0);
    softspi_set_pin(SOFTSPI_DAT, 0);
    usleep(200000);
    softspi_set_pin(SOFTSPI_CS, 1);

    rapidfire_set_channel(g_setting.module.channel-1);
	usleep(100000);
    send_mode(g_setting.module.setting);
}

static void rapidfire_close() {
    softspi_close();
}

// CRC function for IMRC rapidfire API
// Input: byte array, array length
// Output: crc byte
static uint8_t crc8(uint8_t* buf, uint8_t bufLen) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < bufLen; ++i) {
    sum += buf[i];
  }
  return sum & 0xFF;
}

static void send_spi(uint8_t* buf, uint8_t len) {
    softspi_set_pin(SOFTSPI_CS, 0);
    usleep(10);

    for (int i = 0; i < len; ++i) {
        uint8_t byte = buf[i];
        for (uint8_t k = 0; k < 8; k++) {
            softspi_set_pin(SOFTSPI_CLK, 0);
            usleep(10);
            softspi_set_pin(SOFTSPI_DAT, byte & 0x80);
            usleep(10);
            softspi_set_pin(SOFTSPI_CLK, 1);
            usleep(10);
            byte <<= 1;
        }
    }
    softspi_set_pin(SOFTSPI_CS, 1);
    softspi_set_pin(SOFTSPI_CLK, 0);
}

static void send_band(uint8_t band) {
    // convert standard band index to IMRC band index:
    const uint8_t band_map[] = {5, 4, 3, 1, 2, 6, 7};
    uint8_t imrcBand = band_map[band];

    uint8_t cmd[5];
    cmd[0] = RF_API_BAND_CMD;       // 'B'
    cmd[1] = RF_API_DIR_EQUAL;      // '='
    cmd[2] = 0x01;                  // len
    cmd[3] = imrcBand;              // temporarily set byte 4 to band for crc calc
    cmd[3] = crc8(cmd, 4);          // reset byte 4 to crc
    cmd[4] = imrcBand;              // assign band to correct byte 5

    // rapidfire sometimes misses pkts, so send each one 3x
    for (int i = 0; i < SPAM_COUNT; i++) {
        send_spi(cmd, 5);
        usleep(1000);
    }
}

static void send_channel(uint8_t channel) {
    // imrc channel is 1 based
    channel++;

    uint8_t cmd[5];
    cmd[0] = RF_API_CHANNEL_CMD;    // 'C'
    cmd[1] = RF_API_DIR_EQUAL;      // '='
    cmd[2] = 0x01;                  // len
    cmd[3] = channel;               // temporarily set byte 4 to channel for crc calc
    cmd[3] = crc8(cmd, 4);          // reset byte 4 to crc
    cmd[4] = channel;               // assign channel to correct byte 5

    // rapidfire sometimes misses pkts, so send each one 3x
    for (int i = 0; i < SPAM_COUNT; i++) {
        send_spi(cmd, 5);
        usleep(1000);
    }
}

static void send_mode(uint8_t mode) {
    uint8_t cmd[5];
    cmd[0] = RF_API_MODE_CMD;       // 'D'
    cmd[1] = RF_API_DIR_EQUAL;      // '='
    cmd[2] = 0x01;                  // len
    cmd[3] = mode;                  // temporarily set byte 4 to mode for crc calc
    cmd[3] = crc8(cmd, 4);          // reset byte 4 to crc
    cmd[4] = mode;                  // assign mode to correct byte 5

    // rapidfire sometimes misses pkts, so send each one 3x
    for (int i = 0; i < SPAM_COUNT; i++) {
        send_spi(cmd, 5);
        usleep(1000);
    }
}

static char *rapidfire_channels[] = {
    "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8",
    "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8",
    "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
    "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8",
    "L1", "L2", "L3", "L4", "L5", "L6", "L7", "L8",
    "X1", "X2", "X3", "X4", "X5", "X6", "X7", "X8",
};

char *rapidfire_channel_name(int channel) {
    return rapidfire_channels[channel];
}

module_def_t rapidfire_module = {
    56,
    rapidfire_channel_name,
    rapidfire_init,
    rapidfire_close,
    rapidfire_set_channel
};