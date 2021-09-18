#ifdef _WINDOW_CONFIG

/* default window dimensions (overwritten via -g option): */
static const int WIN_WIDTH  = 800;
static const int WIN_HEIGHT = 600;

/* colors and font are configured via X resource properties.
 * See nsxiv(1), X(7) section Resources and xrdb(1) for more information.
 */

#endif
#ifdef _TITLE_CONFIG

/* default title prefix */
static const char *TITLE_PREFIX = "nsxiv - ";

/* default title suffixmode, available options are:
 * SUFFIX_EMPTY
 * SUFFIX_BASENAME
 * SUFFIX_FULLPATH
 */
static const suffixmode_t TITLE_SUFFIXMODE = SUFFIX_BASENAME;

#endif
#ifdef _IMAGE_CONFIG

/* zoom level of 1.0 means 100% */
static const float ZOOM_MIN  = 0.01;
static const float ZOOM_MAX  = 20.0;
static const float ZOOM_STEP = 1.2599210498948732; /* 2^(1/3) */

/* default slideshow delay (in sec, overwritten via -S option): */
static const int SLIDESHOW_DELAY = 5;

/* gamma correction: the user-visible ranges [-GAMMA_RANGE, 0] and
 * (0, GAMMA_RANGE] are mapped to the ranges [0, 1], and (1, GAMMA_MAX].
 * */
static const double GAMMA_MAX   = 10.0;
static const int    GAMMA_RANGE = 32;

/* command i_scroll pans image 1/PAN_FRACTION of screen width/height */
static const int PAN_FRACTION = 5;

/* if false, pixelate images at zoom level != 100%,
 * toggled with 'a' key binding
 */
static const bool ANTI_ALIAS = true;

/* if true, use a checkerboard background for alpha layer,
 * toggled with 'A' key binding
 */
static const bool ALPHA_LAYER = false;

#endif
#ifdef _THUMBS_CONFIG

/* thumbnail sizes in pixels (width == height): */
static const int thumb_sizes[] = { 32, 64, 96, 128, 160 };

/* thumbnail size at startup, index into thumb_sizes[]: */
static const int THUMB_SIZE = 3;

#endif
#ifdef _MAPPINGS_CONFIG

/* following modifiers (NumLock | CapsLock) will be ignored when processing keybindings */
static const int ignore_mask = Mod2Mask | LockMask;

/* abort the keyhandler */
static const KeySym keyhandler_abort = XK_Escape;

/* keyboard mappings for image and thumbnail mode: */
static const keymap_t keys[] = {
	/* modifiers    key               function              argument */
	{ 0,            XK_q,             g_quit,               None },
	{ 0,            XK_Return,        g_switch_mode,        None },
	{ 0,            XK_f,             g_toggle_fullscreen,  None },
	{ 0,            XK_b,             g_toggle_bar,         None },
	{ ControlMask,  XK_x,             g_prefix_external,    None },
	{ 0,            XK_g,             g_first,              None },
	{ ShiftMask,    XK_G,             g_n_or_last,          None },
	{ 0,            XK_r,             g_reload_image,       None },
	{ ShiftMask,    XK_D,             g_remove_image,       None },
	{ ControlMask,  XK_h,             g_scroll_screen,      DIR_LEFT },
	{ ControlMask,  XK_Left,          g_scroll_screen,      DIR_LEFT },
	{ ControlMask,  XK_j,             g_scroll_screen,      DIR_DOWN },
	{ ControlMask,  XK_Down,          g_scroll_screen,      DIR_DOWN },
	{ ControlMask,  XK_k,             g_scroll_screen,      DIR_UP },
	{ ControlMask,  XK_Up,            g_scroll_screen,      DIR_UP },
	{ ControlMask,  XK_l,             g_scroll_screen,      DIR_RIGHT },
	{ ControlMask,  XK_Right,         g_scroll_screen,      DIR_RIGHT },
	{ 0,            XK_plus,          g_zoom,               +1 },
	{ 0,            XK_KP_Add,        g_zoom,               +1 },
	{ 0,            XK_minus,         g_zoom,               -1 },
	{ 0,            XK_KP_Subtract,   g_zoom,               -1 },
	{ 0,            XK_m,             g_toggle_image_mark,  None },
	{ ShiftMask,    XK_M,             g_mark_range,         None },
	{ ControlMask,  XK_m,             g_reverse_marks,      None },
	{ ControlMask,  XK_u,             g_unmark_all,         None },
	{ ShiftMask,    XK_N,             g_navigate_marked,    +1 },
	{ ShiftMask,    XK_P,             g_navigate_marked,    -1 },
	{ 0,            XK_braceleft,     g_change_gamma,       -1 },
	{ 0,            XK_braceright,    g_change_gamma,       +1 },
	{ ControlMask,  XK_g,             g_change_gamma,        0 },

	{ 0,            XK_h,             t_move_sel,           DIR_LEFT },
	{ 0,            XK_Left,          t_move_sel,           DIR_LEFT },
	{ 0,            XK_j,             t_move_sel,           DIR_DOWN },
	{ 0,            XK_Down,          t_move_sel,           DIR_DOWN },
	{ 0,            XK_k,             t_move_sel,           DIR_UP },
	{ 0,            XK_Up,            t_move_sel,           DIR_UP },
	{ 0,            XK_l,             t_move_sel,           DIR_RIGHT },
	{ 0,            XK_Right,         t_move_sel,           DIR_RIGHT },
	{ ShiftMask,    XK_R,             t_reload_all,         None },

	{ 0,            XK_n,             i_navigate,           +1 },
	{ 0,            XK_n,             i_scroll_to_edge,     DIR_LEFT | DIR_UP },
	{ 0,            XK_space,         i_navigate,           +1 },
	{ 0,            XK_p,             i_navigate,           -1 },
	{ 0,            XK_p,             i_scroll_to_edge,     DIR_LEFT | DIR_UP },
	{ 0,            XK_BackSpace,     i_navigate,           -1 },
	{ 0,            XK_bracketright,  i_navigate,           +10 },
	{ 0,            XK_bracketleft,   i_navigate,           -10 },
	{ ControlMask,  XK_6,             i_alternate,          None },
	{ ControlMask,  XK_n,             i_navigate_frame,     +1 },
	{ ControlMask,  XK_p,             i_navigate_frame,     -1 },
	{ ControlMask,  XK_space,         i_toggle_animation,   None },
	{ ControlMask,  XK_a,             i_toggle_animation,   None },
	{ 0,            XK_h,             i_scroll,             DIR_LEFT },
	{ 0,            XK_Left,          i_scroll,             DIR_LEFT },
	{ 0,            XK_j,             i_scroll,             DIR_DOWN },
	{ 0,            XK_Down,          i_scroll,             DIR_DOWN },
	{ 0,            XK_k,             i_scroll,             DIR_UP },
	{ 0,            XK_Up,            i_scroll,             DIR_UP },
	{ 0,            XK_l,             i_scroll,             DIR_RIGHT },
	{ 0,            XK_Right,         i_scroll,             DIR_RIGHT },
	{ ShiftMask,    XK_H,             i_scroll_to_edge,     DIR_LEFT },
	{ ShiftMask,    XK_J,             i_scroll_to_edge,     DIR_DOWN },
	{ ShiftMask,    XK_K,             i_scroll_to_edge,     DIR_UP },
	{ ShiftMask,    XK_L,             i_scroll_to_edge,     DIR_RIGHT },
	{ 0,            XK_equal,         i_set_zoom,           100 },
	{ 0,            XK_w,             i_fit_to_win,         SCALE_DOWN },
	{ ShiftMask,    XK_W,             i_fit_to_win,         SCALE_FIT },
	{ ShiftMask,    XK_F,             i_fit_to_win,         SCALE_FILL },
	{ 0,            XK_e,             i_fit_to_win,         SCALE_WIDTH },
	{ ShiftMask,    XK_E,             i_fit_to_win,         SCALE_HEIGHT },
	{ ShiftMask,    XK_less,          i_rotate,             DEGREE_270 },
	{ ShiftMask,    XK_greater,       i_rotate,             DEGREE_90 },
	{ ShiftMask,    XK_question,      i_rotate,             DEGREE_180 },
	{ ShiftMask,    XK_bar,           i_flip,               FLIP_HORIZONTAL },
	{ ShiftMask,    XK_underscore,    i_flip,               FLIP_VERTICAL },
	{ 0,            XK_a,             i_toggle_antialias,   None },
	{ ShiftMask,    XK_A,             i_toggle_alpha,       None },
	{ 0,            XK_s,             i_slideshow,          None },
};

/* mouse button mappings for image mode: */
static const button_t buttons[] = {
	/* modifiers    button            function              argument */
	{ 0,            1,                i_cursor_navigate,    None },
	{ ControlMask,  1,                i_drag,               DRAG_RELATIVE },
	{ 0,            2,                i_drag,               DRAG_ABSOLUTE },
	{ 0,            3,                g_switch_mode,        None },
	{ 0,            4,                g_zoom,               +1 },
	{ 0,            5,                g_zoom,               -1 },
};

/* mouse cursor on left, middle and right part of the window */
static const cursor_t imgcursor[3] = {
	CURSOR_LEFT, CURSOR_ARROW, CURSOR_RIGHT
};

#endif
