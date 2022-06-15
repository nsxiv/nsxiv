#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdbool.h>

/* global */
bool cg_change_gamma(arg_t);
bool cg_first(arg_t);
bool cg_mark_range(arg_t);
bool cg_n_or_last(arg_t);
bool cg_navigate_marked(arg_t);
bool cg_prefix_external(arg_t);
bool cg_quit(arg_t);
bool cg_reload_image(arg_t);
bool cg_remove_image(arg_t);
bool cg_reverse_marks(arg_t);
bool cg_scroll_screen(arg_t);
bool cg_switch_mode(arg_t);
bool cg_toggle_bar(arg_t);
bool cg_toggle_fullscreen(arg_t);
bool cg_toggle_image_mark(arg_t);
bool cg_unmark_all(arg_t);
bool cg_zoom(arg_t);
/* image mode */
bool ci_alternate(arg_t);
bool ci_cursor_navigate(arg_t);
bool ci_drag(arg_t);
bool ci_fit_to_win(arg_t);
bool ci_flip(arg_t);
bool ci_navigate(arg_t);
bool ci_navigate_frame(arg_t);
bool ci_rotate(arg_t);
bool ci_scroll(arg_t);
bool ci_scroll_to_center(arg_t);
bool ci_scroll_to_edge(arg_t);
bool ci_set_zoom(arg_t);
bool ci_slideshow(arg_t);
bool ci_toggle_alpha(arg_t);
bool ci_toggle_animation(arg_t);
bool ci_toggle_antialias(arg_t);
/* thumbnails mode */
bool ct_move_sel(arg_t);
bool ct_reload_all(arg_t);
bool ct_scroll(arg_t);
bool ct_drag_mark_image(arg_t);
bool ct_select(arg_t);

#ifdef INCLUDE_MAPPINGS_CONFIG
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
#define i_scroll_to_center { ci_scroll_to_center, MODE_IMAGE }
#define i_scroll_to_edge { ci_scroll_to_edge, MODE_IMAGE }
#define i_set_zoom { ci_set_zoom, MODE_IMAGE }
#define i_slideshow { ci_slideshow, MODE_IMAGE }
#define i_toggle_alpha { ci_toggle_alpha, MODE_IMAGE }
#define i_toggle_animation { ci_toggle_animation, MODE_IMAGE }
#define i_toggle_antialias { ci_toggle_antialias, MODE_IMAGE }

/* thumbnails mode */
#define t_move_sel { ct_move_sel, MODE_THUMB }
#define t_reload_all { ct_reload_all, MODE_THUMB }
#define t_scroll { ct_scroll, MODE_THUMB }
#define t_drag_mark_image { ct_drag_mark_image, MODE_THUMB }
#define t_select { ct_select, MODE_THUMB }

#endif /* _MAPPINGS_CONFIG */
#endif /* COMMANDS_H */
