#include "common.hh"

#include <unistd.h>

#include "rapidfire.h"
#include "softspi.h"

#define SPAM_COUNT          3

#define RF_API_DIR_GRTHAN   0x3E    // '>'
#define RF_API_DIR_EQUAL    0x3D    // '='
#define RF_API_BEEP_CMD     0x53    // 'S'
#define RF_API_CHANNEL_CMD  0x43    // 'C'
#define RF_API_BAND_CMD     0x42    // 'B'

static int current_channel = -1;

static const uint32_t periodMicroSec = 1000;

static void send_band(uint8_t band);
static void send_channel(uint8_t channel);

void rapidfire_init() {
    softspi_init();

    softspi_set_pin(SOFTSPI_CLK, 1);
    softspi_set_pin(SOFTSPI_CS, 1);
    softspi_set_pin(SOFTSPI_DAT, 1);
    usleep(200);
    softspi_set_pin(SOFTSPI_CLK, 0);
    softspi_set_pin(SOFTSPI_CS, 0);
    softspi_set_pin(SOFTSPI_DAT, 0);
    usleep(200);
    softspi_set_pin(SOFTSPI_CS, 1);

    rapidfire_set_channel(32);
}

void rapidfire_close() {
    softspi_close();
}

void rapidfire_set_channel(int index) {
    uint8_t band = index / 8 + 1;
    uint8_t channel = index % 8;

    send_band(band);
	usleep(100);
    send_channel(channel);

    current_channel = index;
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
    usleep(periodMicroSec);

    softspi_set_pin(SOFTSPI_CS, 0);
    usleep(100);

    for (int i = 0; i < len; ++i) {
        uint8_t byte = buf[i];
        for (uint8_t k = 0; k < 8; k++) {
            softspi_set_pin(SOFTSPI_CLK, 0);
            softspi_set_pin(SOFTSPI_DAT, byte & 0x80);
            softspi_set_pin(SOFTSPI_CLK, 1);
            byte <<= 1;
        }
    }

    softspi_set_pin(SOFTSPI_DAT, 0);
    softspi_set_pin(SOFTSPI_CLK, 0);
    softspi_set_pin(SOFTSPI_CS, 1);
}

static void send_band(uint8_t band) {
    // convert standard band index to IMRC band index:
    const uint8_t band_map[] = {5, 4, 3, 1, 2, 6};
    uint8_t imrcBand = band_map[band];

    uint8_t cmd[5];
    cmd[0] = RF_API_BAND_CMD;       // 'C'
    cmd[1] = RF_API_DIR_EQUAL;      // '='
    cmd[2] = 0x01;                  // len
    cmd[3] = imrcBand;              // temporarily set byte 4 to band for crc calc
    cmd[3] = crc8(cmd, 4);          // reset byte 4 to crc
    cmd[4] = imrcBand;              // assign band to correct byte 5

    // rapidfire sometimes misses pkts, so send each one 3x
    for (int i = 0; i < SPAM_COUNT; i++) {
        send_spi(cmd, 5);
    }
}

static void send_channel(uint8_t channel) {
    // channel is zero based, need to add 1
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
    }
}
