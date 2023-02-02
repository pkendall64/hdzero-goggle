#include "ui/ui_porting.h"

#include <endian.h>
#include <stdio.h>

#include <log/log.h>

#include "../core/common.hh"
#include "fbtools.h"
#include "lvgl/lvgl.h"

#define DISP_HOR_RES (1920)
#define DISP_VER_RES (1080)

FBDEV fbdev;

static int h_resolution = DISP_HOR_RES;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_buf[DISP_HOR_RES * DISP_VER_RES * 4];
static lv_disp_drv_t disp_drv;
static lv_disp_t *disp;

static void hdz_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    for (lv_coord_t y = area->y1; y <= area->y2; y++) {
        fbdev.fb_mem_offset = (y * h_resolution + area->x1) * 4;
        fbdev.fb_fix.smem_len = 4 * (area->x2 - area->x1 + 1);
        memcpy(fbdev.fb_mem + fbdev.fb_mem_offset, ((char *)color_p), fbdev.fb_fix.smem_len);
        color_p += (area->x2 - area->x1 + 1);
    }

    fb_sync(&fbdev);

    lv_disp_flush_ready(disp);
}

int lvgl_init_porting(void) {
    memset(&fbdev, 0x0, sizeof(FBDEV));
    strncpy(fbdev.dev, "/dev/fb0", sizeof(fbdev.dev));
    if (fb_open(&fbdev) == -1) {
        LOGE("open frame buffer error");
        return -1;
    }
    LOGI("register disp drv");

    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, DISP_HOR_RES * DISP_VER_RES * 4);

    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = hdz_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp = lv_disp_drv_register(&disp_drv);

    return 0;
}

int lvgl_switch_to_720p(void) {
    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, 1280 * 720 * 4);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = 1280;
    disp_drv.ver_res = 720;
    lv_disp_drv_update(disp, &disp_drv);

    h_resolution = 1280;
    return 0;
}

int lvgl_switch_to_1080p(void) {
    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, 1920 * 1080 * 4);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = 1920;
    disp_drv.ver_res = 1080;
    lv_disp_drv_update(disp, &disp_drv);

    h_resolution = 1920;
    return 0;
}

static uint8_t bmp_header[] = {
    // BMP header
    0x42, 0x4d,             // Magic
    0x8a, 0x90, 0x7e, 0x00, // File length
    0x00, 0x00,             // unused
    0x00, 0x00,             // unused
    0x8a, 0x00, 0x00, 0x00, // pixel data start in file
    // DIB header V5
    0x7c, 0x00, 0x00, 0x00,                                                 // DIB header len
    0x80, 0x07, 0x00, 0x00,                                                 // width
    0x38, 0x04, 0x00, 0x00,                                                 // height
    0x01, 0x00,                                                             // number of color planes
    0x20, 0x00,                                                             // bits per pixel
    0x03, 0x00, 0x00, 0x00,                                                 // BI_BITFIELDS, no pixel array compression used
    0x00, 0x90, 0x7e, 0x00,                                                 // size of raw bitmap data
    0x00, 0x00, 0x00, 0x00,                                                 // horizontal pixels/metre
    0x00, 0x00, 0x00, 0x00,                                                 // vertical pixels/metre
    0x00, 0x00, 0x00, 0x00,                                                 // number of colors in palette
    0x00, 0x00, 0x00, 0x00,                                                 // number of important colors
    0x00, 0x00, 0xff, 0x00,                                                 // RED bits
    0x00, 0xff, 0x00, 0x00,                                                 // GREEN bits
    0xff, 0x00, 0x00, 0x00,                                                 // BLUE bits
    0x00, 0x00, 0x00, 0xff,                                                 // ALPHA bits
    0x42, 0x47, 0x52, 0x73,                                                 // colorspace 'BGRs'
    0x80, 0xc2, 0xf5, 0x28, 0x60, 0xb8, 0x1e, 0x15, 0x20, 0x85, 0xeb, 0x01, // RED CIEXYZ tripple
    0x40, 0x33, 0x33, 0x13, 0x80, 0x66, 0x66, 0x26, 0x40, 0x66, 0x66, 0x06, // GREEN CIEXYZ tripple
    0xa0, 0x99, 0x99, 0x09, 0x3c, 0x0a, 0xd7, 0x03, 0x24, 0x5c, 0x8f, 0x32, // BLUE CIEXYZ tripple
    0x00, 0x00, 0x00, 0x00,                                                 // RED gamma
    0x00, 0x00, 0x00, 0x00,                                                 // GREEN gamma
    0x00, 0x00, 0x00, 0x00,                                                 // BLUE gamma
    0x04, 0x00, 0x00, 0x00,                                                 // intent
    0x00, 0x00, 0x00, 0x00,                                                 // profile data
    0x00, 0x00, 0x00, 0x00,                                                 // profile size
    0x00, 0x00, 0x00, 0x00                                                  // reserved
};

void
lvgl_screenshot() {
    static int counter = 0;
    FILE *outfile;
    char filename[64];

    sprintf(filename, "/mnt/extsd/scr_%04d.bmp", counter++);
    outfile = fopen(filename, "w");
    if (outfile == NULL) {
        return;
    }

    uint32_t bytes = htole32(disp_drv.hor_res * disp_drv.ver_res * 4 + sizeof(bmp_header));
    uint32_t width = htole32(disp_drv.hor_res);
    uint32_t height = htole32(-disp_drv.ver_res); // make it -ve so it's top to bottom
    memcpy(&bmp_header[0x02], &bytes, 4);
    memcpy(&bmp_header[0x12], &width, 4);
    memcpy(&bmp_header[0x16], &height, 4);

    fwrite(bmp_header, 1, sizeof(bmp_header), outfile);
    fwrite(fbdev.fb_mem, 4, disp_drv.hor_res * disp_drv.ver_res, outfile);
    fclose(outfile);
}
