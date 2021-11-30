#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdbool.h>

#define G_CMD(c) bool cg_##c(arg_t); static const cmd_t g_##c = { cg_##c, MODE_ALL };
#define I_CMD(c) bool ci_##c(arg_t); static const cmd_t i_##c = { ci_##c, MODE_IMAGE };
#define T_CMD(c) bool ct_##c(arg_t); static const cmd_t t_##c = { ct_##c, MODE_THUMB };

/* global */
G_CMD(change_gamma)
G_CMD(first)
G_CMD(mark_range)
G_CMD(n_or_last)
G_CMD(navigate_marked)
G_CMD(prefix_external)
G_CMD(quit)
G_CMD(reload_image)
G_CMD(remove_image)
G_CMD(reverse_marks)
G_CMD(scroll_screen)
G_CMD(switch_mode)
G_CMD(toggle_bar)
G_CMD(toggle_fullscreen)
G_CMD(toggle_image_mark)
G_CMD(unmark_all)
G_CMD(zoom)

/* image mode */
I_CMD(alternate)
I_CMD(cursor_navigate)
I_CMD(drag)
I_CMD(fit_to_win)
I_CMD(flip)
I_CMD(navigate)
I_CMD(navigate_frame)
I_CMD(rotate)
I_CMD(scroll)
I_CMD(scroll_to_edge)
I_CMD(set_zoom)
I_CMD(slideshow)
I_CMD(toggle_alpha)
I_CMD(toggle_animation)
I_CMD(toggle_antialias)

/* thumbnails mode */
T_CMD(move_sel)
T_CMD(reload_all)

#endif
