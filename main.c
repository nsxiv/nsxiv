/* Copyright 2011-2020 Bert Muennich
 * Copyright 2021-2022 nsxiv contributors
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

#include "nsxiv.h"
#define INCLUDE_MAPPINGS_CONFIG
#include "commands.h"
#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <X11/XF86keysym.h>
#include <X11/keysym.h>

#define MODMASK(mask) ((mask) & USED_MODMASK)
#define BAR_SEP "  "

#define TV_DIFF(t1,t2) (((t1)->tv_sec  - (t2)->tv_sec ) * 1000 + \
                        ((t1)->tv_usec - (t2)->tv_usec) / 1000)
#define TV_ADD_MSEC(tv,t) {             \
  (tv)->tv_sec  += (t) / 1000;          \
  (tv)->tv_usec += (t) % 1000 * 1000;   \
}

typedef struct {
	int err;
	char *cmd;
} extcmd_t;

/* these are not declared in nsxiv.h, as it causes too many -Wshadow warnings */
arl_t arl;
img_t img;
tns_t tns;
win_t win;

appmode_t mode;
const XButtonEvent *xbutton_ev;

fileinfo_t *files;
int filecnt, fileidx;
int alternate;
int markcnt;
int markidx;

int prefix;
static bool extprefix;

static bool resized = false;

static struct {
	extcmd_t f, ft;
	int fd;
	pid_t pid;
} info;

static struct {
	extcmd_t f;
	bool warned;
} keyhandler;

static struct {
	extcmd_t f;
} wintitle;

bool title_dirty;

static struct {
	timeout_f handler;
	struct timeval when;
	bool active;
} timeouts[] = {
	{ redraw       },
	{ reset_cursor },
	{ slideshow    },
	{ animate      },
	{ clear_resize },
};

/**************************
  function implementations
 **************************/
static void cleanup(void)
{
	img_close(&img, false);
	arl_cleanup(&arl);
	tns_free(&tns);
	win_close(&win);
}

static bool xgetline(char **lineptr, size_t *n)
{
	ssize_t len = getdelim(lineptr, n, options->using_null ? '\0' : '\n', stdin);
	if (!options->using_null && len > 0 && (*lineptr)[len-1] == '\n')
		(*lineptr)[len-1] = '\0';
	return len > 0;
}

static void check_add_file(char *filename, bool given)
{
	char *path;

	if (*filename == '\0')
		return;

	if (access(filename, R_OK) < 0 ||
	    (path = realpath(filename, NULL)) == NULL)
	{
		if (given)
			error(0, errno, "%s", filename);
		return;
	}

	if (fileidx == filecnt) {
		filecnt *= 2;
		files = erealloc(files, filecnt * sizeof(*files));
		memset(&files[filecnt/2], 0, filecnt/2 * sizeof(*files));
	}

	files[fileidx].name = estrdup(filename);
	files[fileidx].path = path;
	if (given)
		files[fileidx].flags |= FF_WARN;
	fileidx++;
}

void remove_file(int n, bool manual)
{
	if (n < 0 || n >= filecnt)
		return;

	if (filecnt == 1) {
		if (!manual)
			fprintf(stderr, "nsxiv: no more files to display, aborting\n");
		exit(manual ? EXIT_SUCCESS : EXIT_FAILURE);
	}
	if (files[n].flags & FF_MARK)
		markcnt--;

	if (files[n].path != files[n].name)
		free((void*) files[n].path);
	free((void*) files[n].name);

	if (n + 1 < filecnt) {
		if (tns.thumbs != NULL) {
			if (tns.thumbs[n].im != NULL) {
				imlib_context_set_image(tns.thumbs[n].im);
				imlib_free_image_and_decache();
			}
			memmove(tns.thumbs + n, tns.thumbs + n + 1, (filecnt - n - 1) *
			        sizeof(*tns.thumbs));
			memset(tns.thumbs + filecnt - 1, 0, sizeof(*tns.thumbs));
		}
		memmove(files + n, files + n + 1, (filecnt - n - 1) * sizeof(*files));
	}
	filecnt--;
	if (fileidx > n || fileidx == filecnt)
		fileidx--;
	if (alternate > n || alternate == filecnt)
		alternate--;
	if (markidx > n || markidx == filecnt)
		markidx--;
}

void set_timeout(timeout_f handler, int time, bool overwrite)
{
	unsigned int i;

	for (i = 0; i < ARRLEN(timeouts); i++) {
		if (timeouts[i].handler == handler) {
			if (!timeouts[i].active || overwrite) {
				gettimeofday(&timeouts[i].when, 0);
				TV_ADD_MSEC(&timeouts[i].when, time);
				timeouts[i].active = true;
			}
			return;
		}
	}
}

void reset_timeout(timeout_f handler)
{
	unsigned int i;

	for (i = 0; i < ARRLEN(timeouts); i++) {
		if (timeouts[i].handler == handler) {
			timeouts[i].active = false;
			return;
		}
	}
}

static bool check_timeouts(int *t)
{
	int i = 0, tdiff, tmin = -1;
	struct timeval now;

	while (i < (int)ARRLEN(timeouts)) {
		if (timeouts[i].active) {
			gettimeofday(&now, 0);
			tdiff = TV_DIFF(&timeouts[i].when, &now);
			if (tdiff <= 0) {
				timeouts[i].active = false;
				if (timeouts[i].handler != NULL)
					timeouts[i].handler();
				i = tmin = -1;
			} else if (tmin < 0 || tdiff < tmin) {
				tmin = tdiff;
			}
		}
		i++;
	}
	if (tmin > 0 && t != NULL)
		*t = tmin;
	return tmin > 0;
}

static size_t get_win_title(char *buf, size_t len)
{
	char *argv[8];
	spawn_t pfd;
	char w[12] = "", h[12] = "", z[12] = "", fidx[12], fcnt[12];
	ssize_t n = -1;

	if (wintitle.f.err || buf == NULL || len == 0)
		return 0;

	if (mode == MODE_IMAGE) {
		snprintf(w, ARRLEN(w), "%d", img.w);
		snprintf(h, ARRLEN(h), "%d", img.h);
		snprintf(z, ARRLEN(z), "%d", (int)(img.zoom * 100));
	}
	snprintf(fidx, ARRLEN(fidx), "%d", fileidx+1);
	snprintf(fcnt, ARRLEN(fcnt), "%d", filecnt);
	construct_argv(argv, ARRLEN(argv), wintitle.f.cmd, files[fileidx].path,
	               fidx, fcnt, w, h, z, NULL);
	pfd = spawn(wintitle.f.cmd, argv, X_READ);
	if (pfd.readfd >= 0) {
		if ((n = read(pfd.readfd, buf, len-1)) > 0)
			buf[n] = '\0';
		close(pfd.readfd);
	}

	return MAX(0, n);
}

void close_info(void)
{
	if (info.fd != -1) {
		kill(info.pid, SIGTERM);
		close(info.fd);
		info.fd = -1;
	}
}

void open_info(void)
{
	spawn_t pfd;
	char w[12] = "", h[12] = "";
	char *argv[6];
	char *cmd = mode == MODE_IMAGE ? info.f.cmd : info.ft.cmd;
	bool ferr = mode == MODE_IMAGE ? info.f.err : info.ft.err;

	if (ferr || info.fd >= 0 || win.bar.h == 0)
		return;
	win.bar.l.buf[0] = '\0';
	if (mode == MODE_IMAGE) {
		snprintf(w, sizeof(w), "%d", img.w);
		snprintf(h, sizeof(h), "%d", img.h);
	}
	construct_argv(argv, ARRLEN(argv), cmd, files[fileidx].name, w, h,
	               files[fileidx].path, NULL);
	pfd = spawn(cmd, argv, X_READ);
	if (pfd.readfd >= 0) {
		fcntl(pfd.readfd, F_SETFL, O_NONBLOCK);
		info.fd = pfd.readfd;
		info.pid = pfd.pid;
	}
}

static void read_info(void)
{
	ssize_t i, n;

	if ((n = read(info.fd, win.bar.l.buf, win.bar.l.size - 1)) > 0) {
		win.bar.l.buf[n] = '\0';
		for (i = 0; i < n; ++i) {
			if (win.bar.l.buf[i] == '\n')
				win.bar.l.buf[i] = ' ';
		}
		win_draw(&win);
	}
	close_info();
}

void load_image(int new)
{
	bool prev = new < fileidx;
	static int current;

	if (new < 0 || new >= filecnt)
		return;

	if (win.xwin != None)
		win_set_cursor(&win, CURSOR_WATCH);
	reset_timeout(slideshow);

	if (new != current)
		alternate = current;

	img_close(&img, false);
	while (!img_load(&img, &files[new])) {
		remove_file(new, false);
		if (new >= filecnt)
			new = filecnt - 1;
		else if (new > 0 && prev)
			new--;
	}
	files[new].flags &= ~FF_WARN;
	fileidx = current = new;

	close_info();
	open_info();
	arl_add(&arl, files[fileidx].path);
	title_dirty = true;

	if (img.multi.cnt > 0 && img.multi.animate)
		set_timeout(animate, img.multi.frames[img.multi.sel].delay, true);
	else
		reset_timeout(animate);
}

bool mark_image(int n, bool on)
{
	markidx = n;
	if (!!(files[n].flags & FF_MARK) != on) {
		files[n].flags ^= FF_MARK;
		markcnt += on ? 1 : -1;
		if (mode == MODE_THUMB)
			tns_mark(&tns, n, on);
		return true;
	}
	return false;
}

static void bar_put(win_bar_t *bar, const char *fmt, ...)
{
	size_t len = bar->size - (bar->p - bar->buf), n;
	va_list ap;

	va_start(ap, fmt);
	n = vsnprintf(bar->p, len, fmt, ap);
	bar->p += MIN(len, n);
	va_end(ap);
}

static void update_info(void)
{
	unsigned int i, fn, fw;
	const char *mark;
	win_bar_t *l = &win.bar.l, *r = &win.bar.r;

	/* update bar contents */
	if (win.bar.h == 0 || extprefix)
		return;
	for (fw = 0, i = filecnt; i > 0; fw++, i /= 10);
	mark = files[fileidx].flags & FF_MARK ? "* " : "";
	l->p = l->buf;
	r->p = r->buf;
	if (mode == MODE_THUMB) {
		if (tns.loadnext < tns.end)
			bar_put(l, "Loading... %0*d", fw, tns.loadnext + 1);
		else if (tns.initnext < filecnt)
			bar_put(l, "Caching... %0*d", fw, tns.initnext + 1);
		else if (info.ft.err)
			strncpy(l->buf, files[fileidx].name, l->size);
		bar_put(r, "%s%0*d/%d", mark, fw, fileidx + 1, filecnt);
	} else {
		bar_put(r, "%s", mark);
		if (img.ss.on) {
			if (img.ss.delay % 10 != 0)
				bar_put(r, "%2.1fs" BAR_SEP, (float)img.ss.delay / 10);
			else
				bar_put(r, "%ds" BAR_SEP, img.ss.delay / 10);
		}
		if (img.gamma)
			bar_put(r, "G%+d" BAR_SEP, img.gamma);
		bar_put(r, "%3d%%" BAR_SEP, (int) (img.zoom * 100.0));
		if (img.multi.cnt > 0) {
			for (fn = 0, i = img.multi.cnt; i > 0; fn++, i /= 10);
			bar_put(r, "%0*d/%d" BAR_SEP, fn, img.multi.sel + 1, img.multi.cnt);
		}
		bar_put(r, "%0*d/%d", fw, fileidx + 1, filecnt);
		if (info.f.err)
			strncpy(l->buf, files[fileidx].name, l->size);
	}
}

int nav_button(void)
{
	int x, y, nw;

	if (NAV_WIDTH == 0)
		return 1;

	win_cursor_pos(&win, &x, &y);
	nw = NAV_IS_REL ? win.w * NAV_WIDTH / 100 : NAV_WIDTH;
	nw = MIN(nw, ((int)win.w + 1) / 2);

	if (x < nw)
		return 0;
	else if (x < (int)win.w - nw)
		return 1;
	else
		return 2;
}

void redraw(void)
{
	int t;

	if (mode == MODE_IMAGE) {
		img_render(&img);
		if (img.ss.on) {
			t = img.ss.delay * 100;
			if (img.multi.cnt > 0 && img.multi.animate)
				t = MAX(t, img.multi.length);
			set_timeout(slideshow, t, false);
		}
	} else {
		tns_render(&tns);
	}
	update_info();
	if (title_dirty) {
		size_t n;
		char buf[512];

		if ((n = get_win_title(buf, sizeof(buf))) > 0)
			win_set_title(&win, buf, n);
		title_dirty = false;
	}
	win_draw(&win);
	reset_timeout(redraw);
	reset_cursor();
}

void reset_cursor(void)
{
	int c;
	unsigned int i;
	cursor_t cursor = CURSOR_NONE;

	if (mode == MODE_IMAGE) {
		for (i = 0; i < ARRLEN(timeouts); i++) {
			if (timeouts[i].handler == reset_cursor) {
				if (timeouts[i].active) {
					c = nav_button();
					c = MAX(fileidx > 0 ? 0 : 1, c);
					c = MIN(fileidx + 1 < filecnt ? 2 : 1, c);
					cursor = imgcursor[c];
				}
				break;
			}
		}
	} else {
		if (tns.loadnext < tns.end || tns.initnext < filecnt)
			cursor = CURSOR_WATCH;
		else
			cursor = CURSOR_ARROW;
	}
	win_set_cursor(&win, cursor);
}

void animate(void)
{
	if (img_frame_animate(&img)) {
		set_timeout(animate, img.multi.frames[img.multi.sel].delay, true);
		redraw();
	}
}

void slideshow(void)
{
	load_image(fileidx + 1 < filecnt ? fileidx + 1 : 0);
	redraw();
}

void clear_resize(void)
{
	resized = false;
}

static Bool is_input_ev(Display *dpy, XEvent *ev, XPointer arg)
{
	return ev->type == ButtonPress || ev->type == KeyPress;
}

void handle_key_handler(bool init)
{
	extprefix = init;
	if (win.bar.h == 0)
		return;
	if (init) {
		close_info();
		snprintf(win.bar.l.buf, win.bar.l.size, "Getting key handler input "
		         "(%s to abort)...", XKeysymToString(KEYHANDLER_ABORT));
	} else { /* abort */
		open_info();
		update_info();
	}
	win_draw(&win);
}

static bool run_key_handler(const char *key, unsigned int mask)
{
	FILE *pfs;
	bool marked = mode == MODE_THUMB && markcnt > 0;
	bool changed = false;
	int f, i;
	int fcnt = marked ? markcnt : 1;
	char kstr[32];
	struct stat *oldst, st;
	XEvent dump;
	char *argv[3];
	spawn_t pfd;

	if (keyhandler.f.err) {
		if (!keyhandler.warned) {
			error(0, keyhandler.f.err, "%s", keyhandler.f.cmd);
			keyhandler.warned = true;
		}
		return false;
	}
	if (key == NULL)
		return false;

	close_info();
	strncpy(win.bar.l.buf, "Running key handler...", win.bar.l.size);
	win_draw(&win);
	win_set_cursor(&win, CURSOR_WATCH);
	setenv("NSXIV_USING_NULL", options->using_null ? "1" : "0", 1);

	snprintf(kstr, sizeof(kstr), "%s%s%s%s",
	         mask & ControlMask ? "C-" : "",
	         mask & Mod1Mask    ? "M-" : "",
	         mask & ShiftMask   ? "S-" : "", key);
	construct_argv(argv, ARRLEN(argv), keyhandler.f.cmd, kstr, NULL);
	pfd = spawn(keyhandler.f.cmd, argv, X_WRITE);
	if (pfd.writefd < 0)
		return false;
	if ((pfs = fdopen(pfd.writefd, "w")) == NULL) {
		error(0, errno, "open pipe");
		close(pfd.writefd);
		return false;
	}

	oldst = emalloc(fcnt * sizeof(*oldst));
	for (f = i = 0; f < fcnt; i++) {
		if ((marked && (files[i].flags & FF_MARK)) || (!marked && i == fileidx)) {
			stat(files[i].path, &oldst[f]);
			fprintf(pfs, "%s%c", files[i].name, options->using_null ? '\0' : '\n');
			f++;
		}
	}
	fclose(pfs);
	while (waitpid(pfd.pid, NULL, 0) == -1 && errno == EINTR);

	for (f = i = 0; f < fcnt; i++) {
		if ((marked && (files[i].flags & FF_MARK)) || (!marked && i == fileidx)) {
			if (stat(files[i].path, &st) != 0 ||
			    memcmp(&oldst[f].st_mtime, &st.st_mtime, sizeof(st.st_mtime)) != 0)
			{
				if (tns.thumbs != NULL) {
					tns_unload(&tns, i);
					tns.loadnext = MIN(tns.loadnext, i);
				}
				changed = true;
			}
			f++;
		}
	}
	/* drop user input events that occurred while running the key handler */
	while (XCheckIfEvent(win.env.dpy, &dump, is_input_ev, NULL));

	if (mode == MODE_IMAGE && changed) {
		img_close(&img, true);
		load_image(fileidx);
	} else {
		open_info();
	}
	free(oldst);
	reset_cursor();
	return true;
}

static bool process_bindings(const keymap_t *bindings, unsigned int len, KeySym ksym_or_button,
                             unsigned int state, unsigned int implicit_mod)
{
	unsigned int i;
	bool dirty = false;

	for (i = 0; i < len; i++) {
		if (bindings[i].ksym_or_button == ksym_or_button &&
		    MODMASK(bindings[i].mask | implicit_mod) == MODMASK(state) &&
		    bindings[i].cmd.func != NULL &&
		    (bindings[i].cmd.mode == MODE_ALL || bindings[i].cmd.mode == mode))
		{
			if (bindings[i].cmd.func(bindings[i].arg))
				dirty = true;
		}
	}
	return dirty;
}

static void on_keypress(XKeyEvent *kev)
{
	unsigned int sh = 0;
	KeySym ksym, shksym;
	char dummy, key;
	bool dirty = false;

	XLookupString(kev, &key, 1, &ksym, NULL);

	if (kev->state & ShiftMask) {
		kev->state &= ~ShiftMask;
		XLookupString(kev, &dummy, 1, &shksym, NULL);
		kev->state |= ShiftMask;
		if (ksym != shksym)
			sh = ShiftMask;
	}
	if (IsModifierKey(ksym))
		return;
	if (extprefix && ksym == KEYHANDLER_ABORT && MODMASK(kev->state) == 0) {
		handle_key_handler(false);
	} else if (extprefix) {
		if ((dirty = run_key_handler(XKeysymToString(ksym), kev->state & ~sh)))
			extprefix = false;
		else
			handle_key_handler(false);
	} else if (key >= '0' && key <= '9') {
		/* number prefix for commands */
		prefix = prefix * 10 + (int) (key - '0');
		return;
	} else {
		dirty = process_bindings(keys, ARRLEN(keys), ksym, kev->state, sh);
	}
	if (dirty)
		redraw();
	prefix = 0;
}

static void on_buttonpress(const XButtonEvent *bev)
{
	bool dirty = false;

	if (mode == MODE_IMAGE) {
		set_timeout(reset_cursor, TO_CURSOR_HIDE, true);
		reset_cursor();
		dirty = process_bindings(buttons_img, ARRLEN(buttons_img), bev->button, bev->state, 0);
	} else { /* thumbnail mode */
		dirty = process_bindings(buttons_tns, ARRLEN(buttons_tns), bev->button, bev->state, 0);
	}
	if (dirty)
		redraw();
	prefix = 0;
}

static void run(void)
{
	enum { FD_X, FD_INFO, FD_ARL, FD_CNT };
	struct pollfd pfd[FD_CNT];
	int timeout = 0;
	const struct timespec ten_ms = {0, 10000000};
	bool discard, init_thumb, load_thumb, to_set;
	XEvent ev, nextev;

	xbutton_ev = &ev.xbutton;
	while (true) {
		to_set = check_timeouts(&timeout);
		init_thumb = mode == MODE_THUMB && tns.initnext < filecnt;
		load_thumb = mode == MODE_THUMB && tns.loadnext < tns.end;

		if ((init_thumb || load_thumb || to_set || info.fd != -1 ||
		     arl.fd != -1) && XPending(win.env.dpy) == 0)
		{
			if (load_thumb) {
				set_timeout(redraw, TO_REDRAW_THUMBS, false);
				if (!tns_load(&tns, tns.loadnext, false, false)) {
					remove_file(tns.loadnext, false);
					tns.dirty = true;
				}
				if (tns.loadnext >= tns.end) {
					open_info();
					redraw();
				}
			} else if (init_thumb) {
				set_timeout(redraw, TO_REDRAW_THUMBS, false);
				if (!tns_load(&tns, tns.initnext, false, true))
					remove_file(tns.initnext, false);
			} else {
				pfd[FD_X].fd = ConnectionNumber(win.env.dpy);
				pfd[FD_INFO].fd = info.fd;
				pfd[FD_ARL].fd = arl.fd;
				pfd[FD_X].events = pfd[FD_INFO].events = pfd[FD_ARL].events = POLLIN;

				if (poll(pfd, ARRLEN(pfd), to_set ? timeout : -1) < 0)
					continue;
				if (pfd[FD_INFO].revents & POLLIN)
					read_info();
				if (pfd[FD_ARL].revents & POLLIN) {
					if (arl_handle(&arl)) {
						/* when too fast, imlib2 can't load the image */
						nanosleep(&ten_ms, NULL);
						img_close(&img, true);
						load_image(fileidx);
						redraw();
					}
				}
			}
			continue;
		}

		do {
			XNextEvent(win.env.dpy, &ev);
			discard = false;
			if (XEventsQueued(win.env.dpy, QueuedAlready) > 0) {
				XPeekEvent(win.env.dpy, &nextev);
				switch (ev.type) {
				case ConfigureNotify:
				case MotionNotify:
					discard = ev.type == nextev.type;
					break;
				case KeyPress:
					discard = (nextev.type == KeyPress || nextev.type == KeyRelease)
					          && ev.xkey.keycode == nextev.xkey.keycode;
					break;
				}
			}
		} while (discard);

		switch (ev.type) { /* handle events */
		case ButtonPress:
			on_buttonpress(&ev.xbutton);
			break;
		case ClientMessage:
			if ((Atom) ev.xclient.data.l[0] == atoms[ATOM_WM_DELETE_WINDOW])
				cg_quit(EXIT_SUCCESS);
			break;
		case DestroyNotify:
			cg_quit(EXIT_FAILURE);
			break;
		case ConfigureNotify:
			if (win_configure(&win, &ev.xconfigure)) {
				if (mode == MODE_IMAGE) {
					img.dirty = true;
					img.checkpan = true;
				} else {
					tns.dirty = true;
				}
				if (!resized) {
					redraw();
					set_timeout(clear_resize, TO_REDRAW_RESIZE, false);
					resized = true;
				} else {
					set_timeout(redraw, TO_REDRAW_RESIZE, false);
				}
			}
			break;
		case KeyPress:
			on_keypress(&ev.xkey);
			break;
		case MotionNotify:
			if (mode == MODE_IMAGE) {
				set_timeout(reset_cursor, TO_CURSOR_HIDE, true);
				reset_cursor();
			}
			break;
		}
	}
}

static int fncmp(const void *a, const void *b)
{
	return strcoll(((fileinfo_t*) a)->name, ((fileinfo_t*) b)->name);
}

static void sigchld(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

static void setup_signal(int sig, void (*handler)(int sig))
{
	struct sigaction sa;

	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(sig, &sa, 0) == -1)
		error(EXIT_FAILURE, errno, "signal %d", sig);
}

int main(int argc, char *argv[])
{
	int i, start;
	size_t n;
	char *filename;
	const char *homedir, *dsuffix = "";
	struct stat fstats;
	r_dir_t dir;

	setup_signal(SIGCHLD, sigchld);
	setup_signal(SIGPIPE, SIG_IGN);

	setlocale(LC_COLLATE, "");

	parse_options(argc, argv);

	if (options->clean_cache) {
		tns_init(&tns, NULL, NULL, NULL, NULL);
		tns_clean_cache();
		exit(EXIT_SUCCESS);
	}

	if (options->filecnt == 0 && !options->from_stdin) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (options->recursive || options->from_stdin)
		filecnt = 1024;
	else
		filecnt = options->filecnt;

	files = ecalloc(filecnt, sizeof(*files));
	fileidx = 0;

	if (options->from_stdin) {
		n = 0;
		filename = NULL;
		while (xgetline(&filename, &n))
			check_add_file(filename, true);
		free(filename);
	}

	for (i = 0; i < options->filecnt; i++) {
		filename = options->filenames[i];

		if (stat(filename, &fstats) < 0) {
			error(0, errno, "%s", filename);
			continue;
		}
		if (!S_ISDIR(fstats.st_mode)) {
			check_add_file(filename, true);
		} else {
			if (r_opendir(&dir, filename, options->recursive) < 0) {
				error(0, errno, "%s", filename);
				continue;
			}
			start = fileidx;
			while ((filename = r_readdir(&dir, true)) != NULL) {
				check_add_file(filename, false);
				free((void*) filename);
			}
			r_closedir(&dir);
			if (fileidx - start > 1)
				qsort(files + start, fileidx - start, sizeof(fileinfo_t), fncmp);
		}
	}

	if (fileidx == 0)
		error(EXIT_FAILURE, 0, "No valid image file given, aborting");

	filecnt = fileidx;
	fileidx = options->startnum < filecnt ? options->startnum : 0;

	win_init(&win);
	img_init(&img, &win);
	arl_init(&arl);

	if ((homedir = getenv("XDG_CONFIG_HOME")) == NULL || homedir[0] == '\0') {
		homedir = getenv("HOME");
		dsuffix = "/.config";
	}
	if (homedir != NULL) {
		extcmd_t *cmd[] = { &info.f, &info.ft, &keyhandler.f, &wintitle.f };
		const char *name[] = { "image-info", "thumb-info", "key-handler", "win-title" };
		const char *s = "/nsxiv/exec/";

		for (i = 0; i < (int)ARRLEN(cmd); i++) {
			n = strlen(homedir) + strlen(dsuffix) + strlen(s) + strlen(name[i]) + 1;
			cmd[i]->cmd = emalloc(n);
			snprintf(cmd[i]->cmd, n, "%s%s%s%s", homedir, dsuffix, s, name[i]);
			if (access(cmd[i]->cmd, X_OK) != 0)
				cmd[i]->err = errno;
		}
	} else {
		error(0, 0, "Exec directory not found");
	}
	info.fd = -1;

	if (options->thumb_mode) {
		mode = MODE_THUMB;
		tns_init(&tns, files, &filecnt, &fileidx, &win);
		while (!tns_load(&tns, fileidx, false, false))
			remove_file(fileidx, false);
	} else {
		mode = MODE_IMAGE;
		tns.thumbs = NULL;
		load_image(fileidx);
	}
	win_open(&win);
	win_set_cursor(&win, CURSOR_WATCH);

	atexit(cleanup);

	set_timeout(redraw, 25, false);

	run();

	return 0;
}
