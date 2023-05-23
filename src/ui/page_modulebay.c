#include <log/log.h>
#include <minIni.h>

#include "core/common.hh"
#include "core/settings.h"
#include "module/module.h"
#include "page_common.h"
#include "page_modulebay.h"
#include "ui/ui_style.h"

static lv_obj_t *cont;
static lv_obj_t *btn_module_type;
static lv_obj_t *lbl_module_mode;
static lv_obj_t *btn_module_mode;
static lv_obj_t *back_btn1;
static lv_obj_t *back_btn2;

static lv_obj_t *page_item_focused = NULL;

static lv_coord_t col_dsc[] = {160, 120, 240, 240, 240, 100, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};

void show_module_settings(int mode) {
    switch (lv_dropdown_get_selected(btn_module_type)) {
    case MODULE_RAPIDFIRE:
        lv_dropdown_set_options(btn_module_mode, "legacy\nrapidFIRE 1\nrapidFIRE 2");
        lv_dropdown_set_selected(btn_module_mode, mode);
        lv_obj_clear_flag(btn_module_mode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lbl_module_mode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(back_btn1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
        pp_modulebay.p_arr.max = 3;
        break;

    case MODULE_STEADYVIEW:
    case MODULE_STEADYVIEW_X:
        lv_dropdown_set_options(btn_module_mode, "Diversity\nMix 1\nMix 2\nMix 3");
        lv_dropdown_set_selected(btn_module_mode, mode);
        //steadyview need repower to switch mode, old HW revision can not power cycle analog module
        if (getHwRevision() >= HW_REV_2 || lv_dropdown_get_selected(btn_module_type) == MODULE_STEADYVIEW_X) {
            lv_obj_clear_flag(btn_module_mode, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(lbl_module_mode, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back_btn1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
            pp_modulebay.p_arr.max = 3;
        } else {
            lv_obj_add_flag(btn_module_mode, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(lbl_module_mode, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(back_btn1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
            pp_modulebay.p_arr.max = 2;
        }
        break;

    default:
        lv_obj_add_flag(btn_module_mode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lbl_module_mode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(back_btn1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
        pp_modulebay.p_arr.max = 2;
        break;
    }
}

static lv_obj_t *page_create(lv_obj_t *parent, panel_arr_t *arr) {
    lv_obj_t *page = lv_menu_page_create(parent, NULL);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(page, 1053, 900);
    lv_obj_add_style(page, &style_subpage, LV_PART_MAIN);
    lv_obj_set_style_pad_top(page, 94, 0);

    lv_obj_t *section = lv_menu_section_create(page);
    lv_obj_add_style(section, &style_submenu, LV_PART_MAIN);
    lv_obj_set_size(section, 1053, 894);

    create_text(NULL, section, false, "Module Bay:", LV_MENU_ITEM_BUILDER_VARIANT_2);

    cont = lv_obj_create(section);
    lv_obj_set_size(cont, 1040, 600);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, &style_context, LV_PART_MAIN);

    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);

    create_select_item(arr, cont);

    // create menu entries
    create_label_item(cont, "Type", 1, 0, 1);
    btn_module_type = create_dropdown_item(cont, "Unknown\nSwitch\nrapidFIRE\nSteadyview\nSteadyview-X", 2, 0);
    lv_obj_set_size(btn_module_type, 240, 40);

    // settings/mode menu entries
    lbl_module_mode = create_label_item(cont, "Mode", 1, 1, 1);
    btn_module_mode = create_dropdown_item(cont, "None", 2, 1);
    lv_obj_set_size(btn_module_mode, 240, 40);

    // back buttons in possible locations
    back_btn1 = create_label_item(cont, "< Back", 1, 1, 1);
    back_btn2 = create_label_item(cont, "< Back", 1, 2, 1);

    lv_dropdown_set_selected(btn_module_type, g_setting.module.type);
    show_module_settings(g_setting.module.setting);

    return page;
}

static void page_on_roller(uint8_t key) {
    if (page_item_focused) {
        lv_obj_remove_style(page_item_focused, &style_dropdown, LV_PART_MAIN);

        if (key == DIAL_KEY_UP) {
            uint32_t evt = LV_KEY_DOWN;
            lv_event_send(page_item_focused, LV_EVENT_KEY, &evt);
        } else if (key == DIAL_KEY_DOWN) {
            uint32_t evt = LV_KEY_UP;
            lv_event_send(page_item_focused, LV_EVENT_KEY, &evt);
        }
    }
}

static void page_on_click(uint8_t key, int sel) {
    switch (sel) {

    case 0:
        if (!page_item_focused) {
            lv_obj_t *list = lv_dropdown_get_list(btn_module_type);
            lv_dropdown_open(btn_module_type);
            lv_obj_add_style(list, &style_dropdown, LV_PART_MAIN);
            lv_obj_set_style_text_color(list, lv_color_make(0, 0, 0), LV_PART_SELECTED | LV_STATE_CHECKED);
            page_item_focused = btn_module_type;
            pp_modulebay.p_arr.max = 0; // we are now in control of the roller
        } else {
            lv_event_send(btn_module_type, LV_EVENT_RELEASED, NULL);
            page_item_focused = NULL;
            show_module_settings(g_setting.module.type == lv_dropdown_get_selected(btn_module_type) ? g_setting.module.setting : 0);
        }
        break;

    case 1:
        if (!page_item_focused) {
            lv_obj_t *list = lv_dropdown_get_list(btn_module_mode);
            lv_dropdown_open(btn_module_mode);
            lv_obj_add_style(list, &style_dropdown, LV_PART_MAIN);
            lv_obj_set_style_text_color(list, lv_color_make(0, 0, 0), LV_PART_SELECTED | LV_STATE_CHECKED);
            page_item_focused = btn_module_mode;
            pp_modulebay.p_arr.max = 0; // we are now in control of the roller
        } else {
            lv_event_send(btn_module_mode, LV_EVENT_RELEASED, NULL);
            page_item_focused = NULL;
            show_module_settings(lv_dropdown_get_selected(btn_module_mode));
        }
        break;

    default:
        break;
    }
}

static void page_exit() {
    int module_type = lv_dropdown_get_selected(btn_module_type);
    if (g_setting.module.type != module_type) {
        module_close();
        module_type = g_setting.module.type;
    }
    g_setting.module.type = lv_dropdown_get_selected(btn_module_type);
    ini_putl("module", "type", g_setting.module.type, SETTING_INI);
    g_setting.module.setting = lv_dropdown_get_selected(btn_module_mode);
    ini_putl("module", "setting", g_setting.module.setting, SETTING_INI);
    if (module_type == g_setting.module.type) {
        module_set_mode();
    } else {
        module_init();
    }
}

page_pack_t pp_modulebay = {
    .p_arr = {
        .cur = 0,
        .max = 0,
    },

    .name = "Module Bay",
    .create = page_create,
    .enter = NULL,
    .exit = page_exit,
    .on_roller = page_on_roller,
    .on_click = page_on_click,
    .on_right_button = NULL,
};