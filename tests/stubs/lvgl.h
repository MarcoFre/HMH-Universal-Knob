#pragma once
#include <cstdint>
struct lv_obj_t {};
#define LV_OBJ_FLAG_HIDDEN 1
#define LV_PART_MAIN 0
#define LV_ANIM_OFF 0
inline int lv_color_hex(uint32_t x) {return (int)x;}
inline void lv_label_set_text(lv_obj_t *,const char *) {}
inline void lv_bar_set_value(lv_obj_t *, int, int) {}
inline void lv_obj_set_style_text_color(lv_obj_t *, int, int) {}
inline void lv_obj_set_style_image_recolor(lv_obj_t *, int, int) {}
inline void lv_obj_set_style_border_color(lv_obj_t *, int, int) {}
inline void lv_obj_set_style_bg_color(lv_obj_t *, int, int) {}
inline void lv_obj_add_flag(lv_obj_t *, int) {}
inline void lv_obj_remove_flag(lv_obj_t *, int) {}
