#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdbool.h>

/* global */
bool cg_change_gamma();
bool cg_first();
bool cg_mark_range();
bool cg_n_or_last();
bool cg_navigate_marked();
bool cg_prefix_external();
bool cg_quit();
bool cg_reload_image();
bool cg_remove_image();
bool cg_reverse_marks();
bool cg_scroll_screen();
bool cg_switch_mode();
bool cg_toggle_bar();
bool cg_toggle_fullscreen();
bool cg_toggle_image_mark();
bool cg_unmark_all();
bool cg_zoom();
/* image mode */
bool ci_alternate();
bool ci_cursor_navigate();
bool ci_drag();
bool ci_fit_to_win();
bool ci_flip();
bool ci_navigate();
bool ci_navigate_frame();
bool ci_rotate();
bool ci_scroll();
bool ci_scroll_to_edge();
bool ci_set_zoom();
bool ci_slideshow();
bool ci_toggle_alpha();
bool ci_toggle_animation();
bool ci_toggle_antialias();
/* thumbnails mode */
bool ct_move_sel();
bool ct_reload_all();

/* global */
#define g_change_gamma { cg_change_gamma, MODE_ALL }
#define g_first { cg_first, MODE_ALL }
#define g_mark_range { cg_mark_range, MODE_ALL }
#define g_n_or_last { cg_n_or_last, MODE_ALL }
#define g_navigate_marked { cg_navigate_marked, MODE_ALL }
#define g_prefix_external { cg_prefix_external, MODE_ALL }
#define g_quit { cg_quit, MODE_ALL }
#define g_reload_image { cg_reload_image, MODE_ALL }
#define g_remove_image { cg_remove_image, MODE_ALL }
#define g_reverse_marks { cg_reverse_marks, MODE_ALL }
#define g_scroll_screen { cg_scroll_screen, MODE_ALL }
#define g_switch_mode { cg_switch_mode, MODE_ALL }
#define g_toggle_bar { cg_toggle_bar, MODE_ALL }
#define g_toggle_fullscreen { cg_toggle_fullscreen, MODE_ALL }
#define g_toggle_image_mark { cg_toggle_image_mark, MODE_ALL }
#define g_unmark_all { cg_unmark_all, MODE_ALL }
#define g_zoom { cg_zoom, MODE_ALL }

/* image mode */
#define i_alternate { ci_alternate, MODE_IMAGE }
#define i_cursor_navigate { ci_cursor_navigate, MODE_IMAGE }
#define i_drag { ci_drag, MODE_IMAGE }
#define i_fit_to_win { ci_fit_to_win, MODE_IMAGE }
#define i_flip { ci_flip, MODE_IMAGE }
#define i_navigate { ci_navigate, MODE_IMAGE }
#define i_navigate_frame { ci_navigate_frame, MODE_IMAGE }
#define i_rotate { ci_rotate, MODE_IMAGE }
#define i_scroll { ci_scroll, MODE_IMAGE }
#define i_scroll_to_edge { ci_scroll_to_edge, MODE_IMAGE }
#define i_set_zoom { ci_set_zoom, MODE_IMAGE }
#define i_slideshow { ci_slideshow, MODE_IMAGE }
#define i_toggle_alpha { ci_toggle_alpha, MODE_IMAGE }
#define i_toggle_animation { ci_toggle_animation, MODE_IMAGE }
#define i_toggle_antialias { ci_toggle_antialias, MODE_IMAGE }

/* thumbnails mode */
#define t_move_sel { ct_move_sel, MODE_THUMB }
#define t_reload_all { ct_reload_all, MODE_THUMB }

#endif
