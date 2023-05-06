#include <log/log.h>
#include <minIni.h>

#include "core/common.hh"
#include "core/settings.h"
#include "module/module.h"
#include "page_common.h"
#include "page_modulebay.h"
#include "ui/ui_style.h"

static lv_obj_t *cont;
static btn_group_t btn_group_module_type;
static btn_group_t btn_group_rapidfire;
static btn_group_t btn_group_steadyview;
static lv_obj_t *back_btn2;
static lv_obj_t *back_btn3;
static lv_obj_t *back_btn4;

static lv_coord_t col_dsc[] = {160, 120, 240, 240, 240, 100, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};
lv_obj_t *label_cell_count;

void show_module_settings() {
    switch (btn_group_get_sel(&btn_group_module_type)) {
    case MODULE_RAPIDFIRE:
        display_btn_group(&btn_group_rapidfire, true);
        display_btn_group(&btn_group_steadyview, false);
        lv_obj_add_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(back_btn3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(back_btn4, LV_OBJ_FLAG_HIDDEN);
        btn_group_set_sel(&btn_group_rapidfire, g_setting.module.setting);
        pp_modulebay.p_arr.max = 4;
        break;

    case MODULE_STEADYVIEW:
    case MODULE_STEADYVIEW_X:
        display_btn_group(&btn_group_rapidfire, false);
        //steadyview need repower to switch mode, old HW revision can not power cycle analog module
        display_btn_group(&btn_group_steadyview, getHwRevision() >= HW_REV_2 || btn_group_get_sel(&btn_group_module_type) == MODULE_STEADYVIEW_X);
        lv_obj_add_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(back_btn3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(back_btn4, LV_OBJ_FLAG_HIDDEN);
        btn_group_set_sel(&btn_group_steadyview, g_setting.module.setting);
        pp_modulebay.p_arr.max = 5;
        break;

    default:
        display_btn_group(&btn_group_rapidfire, false);
        display_btn_group(&btn_group_steadyview, false);
        lv_obj_clear_flag(back_btn2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(back_btn3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(back_btn4, LV_OBJ_FLAG_HIDDEN);
        pp_modulebay.p_arr.max = 3;
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
    create_btn_group_item2(&btn_group_module_type, cont, 5, "Type", "Unknown", "Switch", "rapidFIRE", "Steadyview", "Steadyview-X", NULL, 0);
    btn_group_set_sel(&btn_group_module_type, g_setting.module.type);

    // rapidFIRE Mode - rapidFIRE 1, rapidFIRE 2, Legacy
    create_btn_group_item(&btn_group_rapidfire, cont, 3, "Mode", "legacy", "rapidFIRE  1", "rapidFIRE 2", NULL, 2);
    btn_group_set_sel(&btn_group_rapidfire, g_setting.module.setting);

    // Mix Mode - Diversity, Mix 1, Mix 2, Mix 3
    create_btn_group_item2(&btn_group_steadyview, cont, 4, "Mode", "Diversity", "Mix 1", "Mix 2", "Mix 3", NULL, NULL, 2);
    btn_group_set_sel(&btn_group_steadyview, g_setting.module.setting);

    // back buttons in possible locations
    back_btn2 = create_label_item(cont, "< Back", 1, 2, 1);
    back_btn3 = create_label_item(cont, "< Back", 1, 3, 1);
    back_btn4 = create_label_item(cont, "< Back", 1, 4, 1);

    show_module_settings();

    return page;
}

static void page_on_click(uint8_t key, int sel) {
    switch (sel) {

    case 0:
    case 1:
        btn_group_toggle_sel(&btn_group_module_type);
        g_setting.module.setting = 0;
        show_module_settings();
        break;

    case 2:
        if (btn_group_get_sel(&btn_group_module_type) == MODULE_RAPIDFIRE)
            btn_group_toggle_sel(&btn_group_rapidfire);
        if (btn_group_get_sel(&btn_group_module_type) == MODULE_STEADYVIEW || btn_group_get_sel(&btn_group_module_type) == MODULE_STEADYVIEW_X)
            btn_group_toggle_sel(&btn_group_steadyview);
        break;

    case 3:
        if (btn_group_get_sel(&btn_group_module_type) == MODULE_STEADYVIEW || btn_group_get_sel(&btn_group_module_type) == MODULE_STEADYVIEW_X)
            btn_group_toggle_sel(&btn_group_steadyview);
        break;

    default:
        break;
    }
}

static void page_exit() {
    int module_type = btn_group_get_sel(&btn_group_module_type);
    if (g_setting.module.type != module_type) {
        module_close();
        module_type = g_setting.module.type;
    }
    g_setting.module.type = btn_group_get_sel(&btn_group_module_type);
    ini_putl("module", "type", g_setting.module.type, SETTING_INI);
    if (g_setting.module.type == MODULE_RAPIDFIRE)
        g_setting.module.setting = btn_group_get_sel(&btn_group_rapidfire);
    if (g_setting.module.type == MODULE_STEADYVIEW || g_setting.module.type == MODULE_STEADYVIEW_X)
        g_setting.module.setting = btn_group_get_sel(&btn_group_steadyview);
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
        .max = 3,
    },

    .create = page_create,
    .enter = NULL,
    .exit = page_exit,
    .on_roller = NULL,
    .on_click = page_on_click,
    .on_right_button = NULL,
};