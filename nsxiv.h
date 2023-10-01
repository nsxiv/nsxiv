/* Copyright 2011-2020 Bert Muennich
 * Copyright 2021-2023 nsxiv contributors
 *
 * This file is a part of nsxiv.
 *
 * nsxiv is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * nsxiv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nsxiv.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NSXIV_H
#define NSXIV_H

#if !defined(DEBUG) && !defined(NDEBUG)
	#define NDEBUG
#endif

#include <stdbool.h>
#include <stddef.h>

#include <Imlib2.h>
#include <X11/Xlib.h>

/*
 * Annotation for functions called in cleanup().
 * These functions are not allowed to call error(!0, ...) or exit().
 */
#define CLEANUP

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : -(a))

#define ARRLEN(a) (sizeof(a) / sizeof((a)[0]))
#define STREQ(s1,s2) (strcmp((s1), (s2)) == 0)

typedef enum {
	MODE_ALL,
	MODE_IMAGE,
	MODE_THUMB
} appmode_t;

typedef enum {
	DIR_LEFT  = 1,
	DIR_RIGHT = 2,
	DIR_UP    = 4,
	DIR_DOWN  = 8
} direction_t;

typedef enum {
	DEGREE_90  = 1,
	DEGREE_180 = 2,
	DEGREE_270 = 3
} degree_t;

typedef enum {
	FLIP_HORIZONTAL = 1,
	FLIP_VERTICAL   = 2
} flipdir_t;

typedef enum {
	SCALE_DOWN,
	SCALE_FIT,
	SCALE_FILL,
	SCALE_WIDTH,
	SCALE_HEIGHT,
	SCALE_ZOOM
} scalemode_t;

typedef enum {
	DRAG_RELATIVE,
	DRAG_ABSOLUTE
} dragmode_t;

typedef enum {
	CURSOR_ARROW,
	CURSOR_DRAG_ABSOLUTE,
	CURSOR_DRAG_RELATIVE,
	CURSOR_WATCH,
	CURSOR_LEFT,
	CURSOR_RIGHT,
	CURSOR_NONE,

	CURSOR_COUNT
} cursor_t;

typedef enum {
	FF_WARN    = 1,
	FF_MARK    = 2,
	FF_TN_INIT = 4
} fileflags_t;

typedef struct {
	const char *name; /* as given by user */
	const char *path; /* always absolute */
	fileflags_t flags;
} fileinfo_t;

/* timeouts in milliseconds: */
enum {
	TO_AUTORELOAD    = 128,
	TO_REDRAW_RESIZE = 75,
	TO_REDRAW_THUMBS = 200,
	TO_CURSOR_HIDE   = 1200,
	TO_DOUBLE_CLICK  = 300
};

typedef void (*timeout_f)(void);

typedef struct arl arl_t;
typedef struct img img_t;
typedef struct opt opt_t;
typedef struct tns tns_t;
typedef struct win win_t;


/* autoreload.c */

struct arl {
	int fd;
	int wd_dir;
	int wd_file;
	const char *filename;
};

void arl_init(arl_t*);
void arl_cleanup(arl_t*);
void arl_add(arl_t*, const char* /* result of realpath(3) */);
bool arl_handle(arl_t*);


/* commands.c */

typedef int arg_t;
typedef bool (*cmd_f)(arg_t);

typedef struct {
	cmd_f func;
	appmode_t mode;
} cmd_t;

typedef struct {
	unsigned int mask;
	KeySym ksym_or_button;
	cmd_t cmd;
	arg_t arg;
} keymap_t;

typedef keymap_t button_t;


/* image.c */

#ifdef IMLIB2_VERSION /* UPGRADE: Imlib2 v1.8.0: remove all HAVE_IMLIB2_MULTI_FRAME ifdefs */
	#if IMLIB2_VERSION >= IMLIB2_VERSION_(1, 8, 0)
		#define HAVE_IMLIB2_MULTI_FRAME 1
	#endif
#endif
#ifndef HAVE_IMLIB2_MULTI_FRAME
	#define HAVE_IMLIB2_MULTI_FRAME 0
#endif

typedef struct {
	Imlib_Image im;
	unsigned int delay;
} img_frame_t;

typedef struct {
	img_frame_t *frames;
	unsigned int cap;
	unsigned int cnt;
	unsigned int sel;
	bool animate;
	unsigned int framedelay;
	int length;
} multi_img_t;

struct img {
	Imlib_Image im;
	int w;
	int h;

	win_t *win;
	float x;
	float y;

	Imlib_Color_Modifier cmod;
	int gamma;
	int brightness;
	int contrast;

	scalemode_t scalemode;
	float zoom;

	bool checkpan;
	bool dirty;
	bool anti_alias;
	bool alpha_layer;
	bool autoreload_pending;

	struct {
		bool on;
		int delay;
	} ss;

	multi_img_t multi;
};

void img_init(img_t*, win_t*);
bool img_load(img_t*, const fileinfo_t*);
CLEANUP void img_free(Imlib_Image, bool);
CLEANUP void img_close(img_t*, bool);
void img_render(img_t*);
bool img_fit_win(img_t*, scalemode_t);
bool img_zoom(img_t*, int);
bool img_zoom_to(img_t*, float);
bool img_pos(img_t*, float, float);
bool img_pan(img_t*, direction_t, int);
bool img_pan_center(img_t*);
bool img_pan_edge(img_t*, direction_t);
void img_rotate(img_t*, degree_t);
void img_flip(img_t*, flipdir_t);
void img_toggle_antialias(img_t*);
void img_update_color_modifiers(img_t*);
bool img_change_color_modifier(img_t*, int, int*);
bool img_frame_navigate(img_t*, int);
bool img_frame_animate(img_t*);
Imlib_Image img_open(const fileinfo_t*);
#if HAVE_LIBEXIF
void exif_auto_orientate(const fileinfo_t*);
#endif


/* options.c */

struct opt {
	/* file list: */
	char **filenames;
	bool from_stdin;
	bool to_stdout;
	bool using_null;
	bool recursive;
	int filecnt;
	int startnum;

	/* image: */
	scalemode_t scalemode;
	float zoom;
	bool animate;
	bool anti_alias;
	bool alpha_layer;
	int gamma;
	unsigned int slideshow;
	int framerate;

	/* window: */
	bool fullscreen;
	bool hide_bar;
	Window embed; /* unsigned long */
	char *geometry;
	char *res_name;

	/* misc flags: */
	bool quiet;
	bool thumb_mode;
	bool clean_cache;
	bool private_mode;
	bool background_cache;
};

extern const opt_t *options;

void print_usage(void);
void parse_options(int, char**);


/* thumbs.c */

typedef struct {
	Imlib_Image im;
	int w;
	int h;
	int x;
	int y;
} thumb_t;

struct tns {
	fileinfo_t *files;
	thumb_t *thumbs;
	const int *cnt;
	int *sel;
	int initnext;
	int loadnext;
	int first, end;
	int r_first, r_end;

	win_t *win;
	int x;
	int y;
	int cols;
	int rows;
	int zl;
	int bw;
	int dim;

	bool dirty;
};

void tns_clean_cache(void);
void tns_init(tns_t*, fileinfo_t*, const int*, int*, win_t*);
CLEANUP void tns_free(tns_t*);
bool tns_load(tns_t*, int, bool, bool);
void tns_unload(tns_t*, int);
void tns_render(tns_t*);
void tns_mark(tns_t*, int, bool);
void tns_highlight(tns_t*, int, bool);
bool tns_move_selection(tns_t*, direction_t, int);
bool tns_scroll(tns_t*, direction_t, bool);
bool tns_zoom(tns_t*, int);
int tns_translate(tns_t*, int, int);


/* util.c */

#include <dirent.h>

typedef struct {
	DIR *dir;
	char *name;
	int d;
	bool recursive;

	char **stack;
	int stcap;
	int stlen;
} r_dir_t;

extern const char *progname;

void* emalloc(size_t);
void* ecalloc(size_t, size_t);
void* erealloc(void*, size_t);
char* estrdup(const char*);
void error(int, int, const char*, ...);
int r_opendir(r_dir_t*, const char*, bool);
int r_closedir(r_dir_t*);
char* r_readdir(r_dir_t*, bool);
int r_mkdir(char*);
void construct_argv(char**, unsigned int, ...);
pid_t spawn(int*, int*, char *const []);


/* window.c */

#include <X11/Xutil.h>
#if HAVE_LIBFONTS
#include <X11/Xft/Xft.h>
#endif

enum {
	ATOM_WM_DELETE_WINDOW,
	ATOM__NET_WM_NAME,
	ATOM__NET_WM_ICON_NAME,
	ATOM__NET_WM_ICON,
	ATOM__NET_WM_STATE,
	ATOM__NET_WM_PID,
	ATOM__NET_WM_STATE_FULLSCREEN,
	ATOM_UTF8_STRING,
	ATOM_WM_NAME,
	ATOM_WM_ICON_NAME,
	ATOM_COUNT
};

typedef struct {
	Display *dpy;
	int scr;
	int scrw, scrh;
	Visual *vis;
	Colormap cmap;
	int depth;
} win_env_t;

typedef struct {
	size_t size;
	char *p;
	char *buf;
} win_bar_t;

struct win {
	Window xwin;
	win_env_t env;

	XColor win_bg;
	XColor win_fg;
	XColor mrk_fg;
#if HAVE_LIBFONTS
	XftColor bar_bg;
	XftColor bar_fg;
#endif

	int x;
	int y;
	unsigned int w;
	unsigned int h; /* = win height - bar height */
	unsigned int bw;

	struct {
		unsigned int w;
		unsigned int h;
		Pixmap pm;
	} buf;

	struct {
		unsigned int h;
		bool top;
		win_bar_t l;
		win_bar_t r;
	} bar;
};

extern Atom atoms[ATOM_COUNT];

void win_init(win_t*);
void win_open(win_t*);
CLEANUP void win_close(win_t*);
bool win_configure(win_t*, XConfigureEvent*);
void win_toggle_fullscreen(win_t*);
void win_toggle_bar(win_t*);
void win_clear(win_t*);
void win_draw(win_t*);
void win_draw_rect(win_t*, int, int, int, int, bool, int, unsigned long);
void win_set_title(win_t*, const char*, size_t);
void win_set_cursor(win_t*, cursor_t);
void win_cursor_pos(win_t*, int*, int*);

/* main.c */

/* timeout handler functions: */
void redraw(void);
void reset_cursor(void);
void animate(void);
void slideshow(void);
void clear_resize(void);

void remove_file(int, bool);
void set_timeout(timeout_f, int, bool);
void reset_timeout(timeout_f);
void close_info(void);
void open_info(void);
void load_image(int);
bool mark_image(int, bool);
int nav_button(void);
void handle_key_handler(bool);

extern appmode_t mode;
extern const XButtonEvent *xbutton_ev;
extern fileinfo_t *files;
extern int filecnt, fileidx;
extern int alternate;
extern int markcnt;
extern int markidx;
extern int prefix;

#endif /* NSXIV_H */
