#include "ui/ui_porting.h"

#include <endian.h>
#include <stdio.h>

#include <log/log.h>

#include "../core/common.hh"
#include "../core/settings.h"
#include "fbtools.h"
#include "lvgl/lvgl.h"

#ifdef EMULATOR_BUILD
#include "SDLaccess.h"
static void *fb1, *fb2;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
#ifndef WINDOW_NAME
#define WINDOW_NAME "HDZero"
#endif
#endif

typedef enum {
    ORBIT_NONE = 0x00,
    ORBIT_D = 0x01,
    ORBIT_R = 0x02,
    ORBIT_U = 0x04,
    ORBIT_L = 0x08,
    ORBIT_FLUSH = 0x10,
} orbit_state_t;

FBDEV fbdev;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_buf[DRAW_HOR_RES_FHD * DRAW_VER_RES_FHD * 4];
static lv_disp_drv_t disp_drv;
static orbit_state_t disp_orbit_state = ORBIT_NONE;
static int disp_orbit_x, disp_orbit_y;
static lv_disp_t *disp;

static void hdz_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    static int orbit_flush = 0;

    // Manipulates the orbiting speed
    if (++orbit_flush % 500 == 0) {
        disp_orbit_state |= ORBIT_FLUSH;
    }

#ifndef EMULATOR_BUILD
    for (lv_coord_t y = area->y1 + disp_orbit_y; y <= area->y2; y++) {
        fbdev.fb_mem_offset = (y * (disp_drv.hor_res - DISP_OVERSCAN) + disp_orbit_x) * 4;
        fbdev.fb_fix.smem_len = 4 * (area->x2 - area->x1 + 1 - DISP_OVERSCAN);
        memcpy(fbdev.fb_mem + fbdev.fb_mem_offset, ((char *)color_p), fbdev.fb_fix.smem_len);
        color_p += (area->x2 - area->x1 + 1);
    }

    fb_sync(&fbdev);
#else
    SDL_LockMutex(global_sdl_mutex);
    SDL_Rect
        src = {
            .x = 0,
            .y = 0,
            .w = disp_drv.hor_res,
            .h = disp_drv.ver_res,
        },
        dst = {
            .x = 0,
            .y = 0,
            .w = disp_drv.hor_res - DISP_OVERSCAN,
            .h = disp_drv.ver_res - DISP_OVERSCAN,
        };

    SDL_UpdateTexture(texture, &src, color_p, disp_drv.hor_res * ((LV_COLOR_DEPTH + 7) / 8));
    src.x = disp_orbit_x;
    src.w = dst.w;
    src.y = disp_orbit_y;
    src.h = dst.h;
    SDL_RenderCopy(renderer, texture, &src, &dst);
    SDL_RenderPresent(renderer);
    SDL_UnlockMutex(global_sdl_mutex);
#endif

    if (disp_orbit_state & ORBIT_FLUSH) {
        disp_orbit_state &= ~ORBIT_FLUSH;

        int pixels = 1 << g_setting.osd.orbit;

        if (disp_orbit_state & ORBIT_D && disp_orbit_y < pixels) {
            disp_orbit_y += 1;
            if (disp_orbit_y == pixels) {
                disp_orbit_state &= ~ORBIT_D;
                disp_orbit_state |= ORBIT_R;
            }
        } else if (disp_orbit_state & ORBIT_R && disp_orbit_x < pixels) {
            disp_orbit_x += 1;
            if (disp_orbit_x == pixels) {
                disp_orbit_state &= ~ORBIT_R;
                disp_orbit_state |= ORBIT_U;
            }
        } else if (disp_orbit_state & ORBIT_U && disp_orbit_y > 0) {
            disp_orbit_y -= 1;
            if (disp_orbit_y == 0) {
                disp_orbit_state &= !ORBIT_U;
                disp_orbit_state |= ORBIT_L;
            }
        } else if (disp_orbit_state & ORBIT_L && disp_orbit_x > 0) {
            disp_orbit_x -= 1;
            if (disp_orbit_x == 0) {
                disp_orbit_state &= ~ORBIT_L;
                disp_orbit_state |= ORBIT_D;
            }
        }
    }

    lv_disp_flush_ready(disp);
}

int lvgl_init_porting() {
#ifndef EMULATOR_BUILD
    memset(&fbdev, 0x0, sizeof(FBDEV));
    strncpy(fbdev.dev, "/dev/fb0", sizeof(fbdev.dev));
    if (fb_open(&fbdev) == -1) {
        LOGE("open frame buffer error");
        return -1;
    }
    LOGI("register disp drv");

    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, DRAW_HOR_RES_FHD * DRAW_VER_RES_FHD * 4);

    lv_disp_drv_init(&disp_drv);
#else
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
    } else {
        LOGI("SDL already initialised.");
    }

    SDL_LockMutex(global_sdl_mutex);
    window = SDL_CreateWindow(WINDOW_NAME,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              DISP_HOR_RES_FHD, DISP_VER_RES_FHD, 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    texture = SDL_CreateTexture(renderer,
                                (LV_COLOR_DEPTH == 32) ? (SDL_PIXELFORMAT_ARGB8888) : (SDL_PIXELFORMAT_RGB565),
                                SDL_TEXTUREACCESS_STREAMING,
                                DRAW_HOR_RES_FHD,
                                DRAW_VER_RES_FHD);
    SDL_UnlockMutex(global_sdl_mutex);

    fb1 = malloc(DRAW_HOR_RES_FHD * DRAW_VER_RES_FHD * ((LV_COLOR_DEPTH + 7) / 8));
    fb2 = malloc(DRAW_HOR_RES_FHD * DRAW_VER_RES_FHD * ((LV_COLOR_DEPTH + 7) / 8));

    lv_disp_draw_buf_init(&draw_buf, fb1, fb2, DRAW_HOR_RES_FHD * DRAW_VER_RES_FHD);
    lv_disp_drv_init(&disp_drv);
#endif

    disp_drv.full_refresh = 1;
    disp_drv.flush_cb = hdz_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = DRAW_HOR_RES_FHD;
    disp_drv.ver_res = DRAW_VER_RES_FHD;
    disp = lv_disp_drv_register(&disp_drv);

    return 0;
}

int lvgl_switch_to_720p() {
    lvgl_screen_orbit(false);
    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, DRAW_HOR_RES_HD * DRAW_VER_RES_HD * 4);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = DRAW_HOR_RES_HD;
    disp_drv.ver_res = DRAW_VER_RES_HD;
    lv_disp_drv_update(disp, &disp_drv);
    lvgl_screen_orbit(g_setting.osd.orbit > 0);
    return 0;
}

int lvgl_switch_to_1080p() {
    lvgl_screen_orbit(false);
    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, DRAW_HOR_RES_FHD * DRAW_VER_RES_FHD * 4);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = DRAW_HOR_RES_FHD;
    disp_drv.ver_res = DRAW_VER_RES_FHD;
    lv_disp_drv_update(disp, &disp_drv);
    lvgl_screen_orbit(g_setting.osd.orbit > 0);
    return 0;
}

void lvgl_screen_orbit(bool enable) {
    if (enable) {
        disp_orbit_state = ORBIT_D;
    } else {
        disp_orbit_state = ORBIT_NONE;
        disp_orbit_x = 0;
        disp_orbit_y = 0;
    }
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
