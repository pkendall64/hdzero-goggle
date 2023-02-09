#include <log/log.h>
#include <minIni.h>

#include "core/common.hh"
#include "core/settings.h"
#include "module/module.h"
#include "page_common.h"
#include "page_modulebay.h"
#include "ui/ui_style.h"

enum {
    ROW_MODULE_TYPE = 0, // tales 2 rows
    ROW_BACK = 2
};

static btn_group_t btn_group_module_type;

static lv_coord_t col_dsc[] = {160, 120, 240, 240, 240, 100, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};
lv_obj_t *label_cell_count;

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

    lv_obj_t *cont = lv_obj_create(section);
    lv_obj_set_size(cont, 1040, 600);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, &style_context, LV_PART_MAIN);

    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);

    create_select_item(arr, cont);

    // create menu entries
    create_btn_group_item2(&btn_group_module_type, cont, 5, "Type", "Unknown", "Switch", "rapidFIRE", "Steadyview", "Steadyview-X", NULL, ROW_MODULE_TYPE);
    btn_group_set_sel(&btn_group_module_type, g_setting.module.type);

    create_label_item(cont, "< Back", 1, ROW_BACK, 1);

    return page;
}

static void page_on_click(uint8_t key, int sel) {

    switch (sel) {

    case ROW_MODULE_TYPE:
    case ROW_MODULE_TYPE+1:
        btn_group_toggle_sel(&btn_group_module_type);
        break;

    default:
        break;
    }
}

static void page_exit() {
    g_setting.module.type = btn_group_get_sel(&btn_group_module_type);
    ini_putl("module", "type", g_setting.module.type, SETTING_INI);
    module_close();
    module_init();
}

page_pack_t pp_modulebay = {
    .p_arr = {
        .cur = 0,
        .max = 3,
    },

    .name = "Module Bay",
    .create = page_create,
    .enter = NULL,
    .exit = page_exit,
    .on_roller = NULL,
    .on_click = page_on_click,
    .on_right_button = NULL,
};