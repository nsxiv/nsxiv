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

#include "nsxiv.h"
#define INCLUDE_IMAGE_CONFIG
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if HAVE_LIBEXIF
#include <libexif/exif-data.h>
#endif

#ifdef IMLIB2_VERSION
	#if IMLIB2_VERSION >= IMLIB2_VERSION_(1, 8, 0)
		#define HAVE_IMLIB2_MULTI_FRAME 1
	#endif
#endif
#ifndef HAVE_IMLIB2_MULTI_FRAME
	#define HAVE_IMLIB2_MULTI_FRAME 0
#endif

#if HAVE_LIBGIF && !HAVE_IMLIB2_MULTI_FRAME
#include <gif_lib.h>
enum { DEF_GIF_DELAY = 75 };
#endif

#if HAVE_LIBWEBP && !HAVE_IMLIB2_MULTI_FRAME
#include <stdio.h>
#include <webp/decode.h>
#include <webp/demux.h>
enum { DEF_WEBP_DELAY = 75 };
#endif

#if HAVE_IMLIB2_MULTI_FRAME
enum { DEF_ANIM_DELAY = 75 };
#endif

#define ZOOM_MIN (zoom_levels[0] / 100)
#define ZOOM_MAX (zoom_levels[ARRLEN(zoom_levels) - 1] / 100)

static int calc_cache_size(void)
{
	long cache, pages = -1, page_size = -1;

	if (CACHE_SIZE_MEM_PERCENTAGE <= 0)
		return 0;
#ifdef _SC_PHYS_PAGES /* _SC_PHYS_PAGES isn't POSIX */
	pages = sysconf(_SC_PHYS_PAGES);
	page_size = sysconf(_SC_PAGE_SIZE);
#endif
	if (pages < 0 || page_size < 0)
		return CACHE_SIZE_FALLBACK;
	cache = (pages / 100) * CACHE_SIZE_MEM_PERCENTAGE;
	cache *= page_size;

	return MIN(cache, CACHE_SIZE_LIMIT);
}

void img_init(img_t *img, win_t *win)
{
	imlib_context_set_display(win->env.dpy);
	imlib_context_set_visual(win->env.vis);
	imlib_context_set_colormap(win->env.cmap);
	imlib_set_cache_size(calc_cache_size());

	img->im = NULL;
	img->win = win;
	img->scalemode = options->scalemode;
	img->zoom = options->zoom;
	img->zoom = MAX(img->zoom, ZOOM_MIN);
	img->zoom = MIN(img->zoom, ZOOM_MAX);
	img->checkpan = false;
	img->dirty = false;
	img->anti_alias = options->anti_alias;
	img->alpha_layer = options->alpha_layer;
	img->multi.cap = img->multi.cnt = 0;
	img->multi.animate = options->animate;
	img->multi.framedelay = options->framerate > 0 ? 1000 / options->framerate : 0;
	img->multi.length = 0;

	img->cmod = imlib_create_color_modifier();
	imlib_context_set_color_modifier(img->cmod);
	img->brightness = 0;
	img->contrast = 0;
	img_change_color_modifier(img, options->gamma, &img->gamma);

	img->ss.on = options->slideshow > 0;
	img->ss.delay = options->slideshow > 0 ? options->slideshow : SLIDESHOW_DELAY * 10u;
}

#if HAVE_LIBEXIF
void exif_auto_orientate(const fileinfo_t *file)
{
	ExifData *ed;
	ExifEntry *entry;
	int byte_order, orientation = 0;

	if ((ed = exif_data_new_from_file(file->path)) == NULL)
		return;
	byte_order = exif_data_get_byte_order(ed);
	entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
	if (entry != NULL)
		orientation = exif_get_short(entry->data, byte_order);
	exif_data_unref(ed);

	switch (orientation) {
	case 5:
		imlib_image_orientate(1);
		/* fall through */
	case 2:
		imlib_image_flip_vertical();
		break;
	case 3:
		imlib_image_orientate(2);
		break;
	case 7:
		imlib_image_orientate(1);
		/* fall through */
	case 4:
		imlib_image_flip_horizontal();
		break;
	case 6:
		imlib_image_orientate(1);
		break;
	case 8:
		imlib_image_orientate(3);
		break;
	}
}
#endif

#if HAVE_LIBGIF || HAVE_LIBWEBP || HAVE_IMLIB2_MULTI_FRAME
static void img_multiframe_context_set(img_t *img)
{
	if (img->multi.cnt > 1) {
		imlib_context_set_image(img->im);
		imlib_free_image();
		img->im = img->multi.frames[0].im;
	} else if (img->multi.cnt == 1) {
		imlib_context_set_image(img->multi.frames[0].im);
		imlib_free_image();
		img->multi.cnt = 0;
	}

	imlib_context_set_image(img->im);
}
#endif

#if (HAVE_LIBGIF || HAVE_LIBWEBP) && !HAVE_IMLIB2_MULTI_FRAME
static void img_multiframe_deprecation_notice(void)
{
	static bool warned;
	if (!warned) {
		error(0, 0, "\n"
		      "################################################################\n"
		      "#                      DEPRECATION NOTICE                      #\n"
		      "################################################################\n"
		      "# Internal multi-frame gif and webp loaders are deprecated and #\n"
		      "# will be removed soon. Please upgrade to Imlib2 v1.8.0 for    #\n"
		      "# multi-frame/animated image support.                          #\n"
		      "################################################################");
		warned = true;
	}
}
#endif

#if HAVE_LIBGIF && !HAVE_IMLIB2_MULTI_FRAME
static bool img_load_gif(img_t *img, const fileinfo_t *file)
{
	GifFileType *gif;
	GifRowType *rows = NULL;
	GifRecordType rec;
	ColorMapObject *cmap;
	uint32_t bgpixel = 0, *data, *ptr;
	uint32_t *prev_frame = NULL;
	Imlib_Image im;
	int i, j, bg, r, g, b;
	int x, y, w, h, sw, sh;
	int px, py, pw, ph;
	int intoffset[] = { 0, 4, 2, 1 };
	int intjump[] = { 8, 8, 4, 2 };
	int transp = -1;
	unsigned int disposal = 0, prev_disposal = 0;
	unsigned int delay = 0;
	bool err = false;
	multi_img_t *m = &img->multi;

	img_multiframe_deprecation_notice();

#if defined(GIFLIB_MAJOR) && GIFLIB_MAJOR >= 5
	gif = DGifOpenFileName(file->path, NULL);
#else
	gif = DGifOpenFileName(file->path);
#endif
	if (gif == NULL) {
		error(0, 0, "%s: Error opening gif image", file->name);
		return false;
	}
	bg = gif->SBackGroundColor;
	sw = gif->SWidth;
	sh = gif->SHeight;
	px = py = pw = ph = 0;

	m->length = m->cnt = m->sel = 0;
	do {
		if (DGifGetRecordType(gif, &rec) == GIF_ERROR) {
			err = true;
			break;
		}
		if (rec == EXTENSION_RECORD_TYPE) {
			int ext_code;
			GifByteType *ext = NULL;

			DGifGetExtension(gif, &ext_code, &ext);
			while (ext) {
				if (ext_code == GRAPHICS_EXT_FUNC_CODE) {
					if (ext[1] & 1)
						transp = (int)ext[4];
					else
						transp = -1;

					delay = 10 * ((unsigned int)ext[3] << 8 | (unsigned int)ext[2]);
					disposal = (unsigned int)ext[1] >> 2 & 0x7;
				}
				ext = NULL;
				DGifGetExtensionNext(gif, &ext);
			}
		} else if (rec == IMAGE_DESC_RECORD_TYPE) {
			if (DGifGetImageDesc(gif) == GIF_ERROR) {
				err = true;
				break;
			}
			x = gif->Image.Left;
			y = gif->Image.Top;
			w = gif->Image.Width;
			h = gif->Image.Height;

			rows = emalloc(h * sizeof(*rows));
			for (i = 0; i < h; i++)
				rows[i] = emalloc(w * sizeof(*rows[i]));
			if (gif->Image.Interlace) {
				for (i = 0; i < 4; i++) {
					for (j = intoffset[i]; j < h; j += intjump[i])
						DGifGetLine(gif, rows[j], w);
				}
			} else {
				for (i = 0; i < h; i++)
					DGifGetLine(gif, rows[i], w);
			}

			ptr = data = emalloc(sw * sh * sizeof(*data));
			cmap = gif->Image.ColorMap ? gif->Image.ColorMap : gif->SColorMap;
			/* if bg > cmap->ColorCount, it is transparent black already */
			if (cmap && bg >= 0 && bg < cmap->ColorCount) {
				r = cmap->Colors[bg].Red;
				g = cmap->Colors[bg].Green;
				b = cmap->Colors[bg].Blue;
				bgpixel = 0x00ffffff & (r << 16 | g << 8 | b);
			}

			for (i = 0; i < sh; i++) {
				for (j = 0; j < sw; j++) {
					if (i < y || i >= y + h || j < x || j >= x + w ||
					    rows[i - y][j - x] == transp)
					{
						if (prev_frame != NULL &&
						    (prev_disposal != 2 || i < py || i >= py + ph ||
						     j < px || j >= px + pw))
						{
							*ptr = prev_frame[i * sw + j];
						} else {
							*ptr = bgpixel;
						}
					} else {
						assert(cmap != NULL);
						r = cmap->Colors[rows[i - y][j - x]].Red;
						g = cmap->Colors[rows[i - y][j - x]].Green;
						b = cmap->Colors[rows[i - y][j - x]].Blue;
						*ptr = 0xffu << 24 | r << 16 | g << 8 | b;
					}
					ptr++;
				}
			}

			im = imlib_create_image_using_copied_data(sw, sh, data);

			for (i = 0; i < h; i++)
				free(rows[i]);
			free(rows);
			free(data);

			if (im == NULL) {
				err = true;
				break;
			}

			imlib_context_set_image(im);
			imlib_image_set_format("gif");
			if (transp >= 0)
				imlib_image_set_has_alpha(1);

			if (disposal != 3)
				prev_frame = imlib_image_get_data_for_reading_only();
			prev_disposal = disposal;
			px = x, py = y, pw = w, ph = h;

			assert(m->cnt <= m->cap);
			if (m->cnt == m->cap) {
				m->cap = m->cap == 0 ? 16 : (m->cap * 2);
				m->frames = erealloc(m->frames, m->cap * sizeof(*m->frames));
			}
			m->frames[m->cnt].im = im;
			delay = m->framedelay > 0 ? m->framedelay : delay;
			m->frames[m->cnt].delay = delay > 0 ? delay : DEF_GIF_DELAY;
			m->length += m->frames[m->cnt].delay;
			m->cnt++;
		}
	} while (rec != TERMINATE_RECORD_TYPE);

#if defined(GIFLIB_MAJOR) && GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
	DGifCloseFile(gif, NULL);
#else
	DGifCloseFile(gif);
#endif

	if (err && (file->flags & FF_WARN))
		error(0, 0, "%s: Corrupted gif file", file->name);

	img_multiframe_context_set(img);

	return !err;
}
#endif /* HAVE_LIBGIF */

#if HAVE_LIBWEBP && !HAVE_IMLIB2_MULTI_FRAME
static bool img_load_webp(img_t *img, const fileinfo_t *file)
{
	FILE *webp_file;
	WebPData data;
	Imlib_Image im = NULL;
	struct WebPAnimDecoderOptions opts;
	WebPAnimDecoder *dec = NULL;
	struct WebPAnimInfo info;
	unsigned char *buf = NULL, *bytes = NULL;
	int ts;
	const WebPDemuxer *demux;
	WebPIterator iter;
	unsigned long flags;
	unsigned int delay;
	bool err = false;
	multi_img_t *m = &img->multi;

	img_multiframe_deprecation_notice();

	if ((webp_file = fopen(file->path, "rb")) == NULL) {
		error(0, errno, "%s: Error opening webp image", file->name);
		return false;
	}
	fseek(webp_file, 0L, SEEK_END);
	data.size = ftell(webp_file);
	rewind(webp_file);
	bytes = emalloc(data.size);
	if ((err = fread(bytes, 1, data.size, webp_file) != data.size)) {
		error(0, 0, "%s: Error reading webp image", file->name);
		goto fail;
	}
	data.bytes = bytes;

	/* Setup the WebP Animation Decoder */
	if ((err = !WebPAnimDecoderOptionsInit(&opts))) {
		error(0, 0, "%s: WebP library version mismatch", file->name);
		goto fail;
	}
	opts.color_mode = MODE_BGRA;
	/* NOTE: Multi-threaded decoding may cause problems on some system */
	opts.use_threads = true;
	dec = WebPAnimDecoderNew(&data, &opts);
	if ((err = (dec == NULL) || !WebPAnimDecoderGetInfo(dec, &info))) {
		error(0, 0, "%s: WebP parsing or memory error (file is corrupt?)", file->name);
		goto fail;
	}
	demux = WebPAnimDecoderGetDemuxer(dec);

	/* Get global information for the image */
	flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);
	img->w = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
	img->h = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);

	if (info.frame_count > m->cap) {
		m->cap = info.frame_count;
		m->frames = erealloc(m->frames, m->cap * sizeof(*m->frames));
	}

	/* Load and decode frames (also works on images with only 1 frame) */
	m->length = m->cnt = m->sel = 0;
	while (WebPAnimDecoderGetNext(dec, &buf, &ts)) {
		im = imlib_create_image_using_copied_data(info.canvas_width, info.canvas_height,
		                                          (uint32_t *)buf);
		imlib_context_set_image(im);
		imlib_image_set_format("webp");
		/* Get an iterator of this frame - used for frame info (duration, etc.) */
		WebPDemuxGetFrame(demux, m->cnt + 1, &iter);
		imlib_image_set_has_alpha((flags & ALPHA_FLAG) == ALPHA_FLAG);
		/* Store info for this frame */
		m->frames[m->cnt].im = im;
		delay = iter.duration > 0 ? iter.duration : DEF_WEBP_DELAY;
		m->frames[m->cnt].delay = delay;
		m->length += m->frames[m->cnt].delay;
		m->cnt++;
	}
	WebPDemuxReleaseIterator(&iter);

	img_multiframe_context_set(img);
fail:
	if (dec != NULL)
		WebPAnimDecoderDelete(dec);
	free(bytes);
	fclose(webp_file);
	return !err;
}
#endif /* HAVE_LIBWEBP */

#if HAVE_IMLIB2_MULTI_FRAME
static void img_area_clear(int x, int y, int w, int h)
{
	assert(x >= 0 && y >= 0);
	assert(w > 0 && h > 0);
	imlib_image_set_has_alpha(1);
	imlib_context_set_blend(0);
	imlib_context_set_color(0, 0, 0, 0);
	imlib_image_fill_rectangle(x, y, w, h);
}

static bool img_load_multiframe(img_t *img, const fileinfo_t *file)
{
	unsigned int n, fcnt;
	Imlib_Image blank;
	Imlib_Frame_Info finfo;
	int px, py, pw, ph, pflag;
	multi_img_t *m = &img->multi;

	imlib_context_set_image(img->im);
	imlib_image_get_frame_info(&finfo);
	if ((fcnt = finfo.frame_count) <= 1 || !(finfo.frame_flags & IMLIB_IMAGE_ANIMATED))
		return false;
	img->w = finfo.canvas_w;
	img->h = finfo.canvas_h;

	if (fcnt > m->cap) {
		m->cap = fcnt;
		m->frames = erealloc(m->frames, m->cap * sizeof(*m->frames));
	}

	if ((blank = imlib_create_image(img->w, img->h)) == NULL) {
		error(0, 0, "%s: couldn't create image", file->name);
		return false;
	}
	imlib_context_set_image(blank);
	img_area_clear(0, 0, img->w, img->h);

	imlib_context_set_dither(0);
	imlib_context_set_anti_alias(0);
	imlib_context_set_color_modifier(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);

	/*
	 * Imlib2 gives back a "raw frame", we need to blend it on top of the
	 * previous frame ourselves if necessary to get the fully decoded frame.
	 */
	pflag = m->length = m->cnt = m->sel = 0;
	px = py = pw = ph = 0;
	for (n = 1; n <= fcnt; ++n) {
		Imlib_Image frame, canvas;
		int sx, sy, sw, sh;
		bool has_alpha;

		imlib_context_set_image(m->cnt < 1 ? blank : m->frames[m->cnt - 1].im);
		if ((canvas = imlib_clone_image()) == NULL ||
		    (frame = imlib_load_image_frame(file->path, n)) == NULL)
		{
			if (canvas != NULL) {
				imlib_context_set_image(canvas);
				imlib_free_image();
			}
			error(0, 0, "%s: failed to load frame %d", file->name, n);
			break;
		}

		imlib_context_set_image(frame);
		imlib_image_set_changes_on_disk(); /* see img_load() for rationale */
		imlib_image_get_frame_info(&finfo);
		assert(finfo.frame_count == (int)fcnt);
		assert(finfo.canvas_w == img->w && finfo.canvas_h == img->h);
		sx = finfo.frame_x;
		sy = finfo.frame_y;
		sw = finfo.frame_w;
		sh = finfo.frame_h;
		has_alpha = imlib_image_has_alpha();

		imlib_context_set_image(canvas);
		/* the dispose flags are explained in Imlib2's header */
		if (pflag & IMLIB_FRAME_DISPOSE_CLEAR) {
			img_area_clear(px, py, pw, ph);
		} else if (pflag & IMLIB_FRAME_DISPOSE_PREV) {
			Imlib_Image p = m->cnt < 2 ? blank : m->frames[m->cnt - 2].im;
			assert(m->cnt > 0);
			img_area_clear(0, 0, img->w, img->h);
			imlib_blend_image_onto_image(p, 1, px, py, pw, ph, px, py, pw, ph);
		}
		pflag = finfo.frame_flags;
		if (pflag & (IMLIB_FRAME_DISPOSE_CLEAR | IMLIB_FRAME_DISPOSE_PREV)) {
			/* remember these so we can "dispose" them before blending next frame */
			px = sx;
			py = sy;
			pw = sw;
			ph = sh;
		}
		assert(imlib_context_get_operation() == IMLIB_OP_COPY);
		imlib_image_set_has_alpha(has_alpha);
		imlib_context_set_blend(!!(finfo.frame_flags & IMLIB_FRAME_BLEND));
		imlib_blend_image_onto_image(frame, has_alpha, 0, 0, sw, sh, sx, sy, sw, sh);
		m->frames[m->cnt].im = canvas;
		m->frames[m->cnt].delay = finfo.frame_delay ? finfo.frame_delay : DEF_ANIM_DELAY;
		m->length += m->frames[m->cnt].delay;
		m->cnt++;
		imlib_context_set_image(frame);
		imlib_free_image();
	}
	imlib_context_set_image(blank);
	imlib_free_image();
	img_multiframe_context_set(img);
	imlib_context_set_color_modifier(img->cmod); /* restore cmod */
	return m->cnt > 0;
}
#endif /* HAVE_IMLIB2_MULTI_FRAME */

Imlib_Image img_open(const fileinfo_t *file)
{
	struct stat st;
	Imlib_Image im = NULL;

	if (access(file->path, R_OK) == 0 &&
	    stat(file->path, &st) == 0 && S_ISREG(st.st_mode) &&
#if HAVE_IMLIB2_MULTI_FRAME
	    (im = imlib_load_image_frame(file->path, 1)) != NULL)
#else
	    (im = imlib_load_image_immediately(file->path)) != NULL)
#endif
	{
		imlib_context_set_image(im);
	}
	if (im == NULL && (file->flags & FF_WARN))
		error(0, 0, "%s: Error opening image", file->name);
	return im;
}

bool img_load(img_t *img, const fileinfo_t *file)
{
	const char *fmt;
	bool animated = false;

	if ((img->im = img_open(file)) == NULL)
		return false;

	/* ensure that the image's timestamp is checked when loading from cache
	 * to avoid issues like: https://codeberg.org/nsxiv/nsxiv/issues/436
	 */
	imlib_image_set_changes_on_disk();

/* since v1.7.5, Imlib2 can parse exif orientation from jpeg files.
 * this version also happens to be the first one which defines the
 * IMLIB2_VERSION macro.
 */
#if HAVE_LIBEXIF && !defined(IMLIB2_VERSION)
	exif_auto_orientate(file);
#endif

#if HAVE_IMLIB2_MULTI_FRAME
	animated = img_load_multiframe(img, file);
#endif

	if ((fmt = imlib_image_format()) != NULL) { /* NOLINT: fmt might be unused, not worth fixing */
#if HAVE_LIBGIF && !HAVE_IMLIB2_MULTI_FRAME
		if (STREQ(fmt, "gif"))
			img_load_gif(img, file);
#endif
#if HAVE_LIBWEBP && !HAVE_IMLIB2_MULTI_FRAME
		if (STREQ(fmt, "webp"))
			img_load_webp(img, file);
#endif
#if HAVE_LIBEXIF && defined(IMLIB2_VERSION)
		if (!STREQ(fmt, "jpeg") && !STREQ(fmt, "jpg"))
			exif_auto_orientate(file);
#endif
	}
	/* for animated images, we want the _canvas_ width/height, which
	 * img_load_multiframe() sets already.
	 */
	if (!animated) {
		img->w = imlib_image_get_width();
		img->h = imlib_image_get_height();
	}
	img->checkpan = true;
	img->dirty = true;

	return true;
}

CLEANUP void img_close(img_t *img, bool decache)
{
	unsigned int i;
	void (*free_img)(void) = decache ? imlib_free_image_and_decache : imlib_free_image;

	if (img->multi.cnt > 0) {
		for (i = 0; i < img->multi.cnt; i++) {
			imlib_context_set_image(img->multi.frames[i].im);
			free_img();
		}
		img->multi.cnt = 0;
		img->im = NULL;
	} else if (img->im != NULL) {
		imlib_context_set_image(img->im);
		free_img();
		img->im = NULL;
	}
}

static void img_check_pan(img_t *img, bool moved)
{
	win_t *win;
	float w, h, ox, oy;

	win = img->win;
	w = img->w * img->zoom;
	h = img->h * img->zoom;
	ox = img->x;
	oy = img->y;

	if (w < win->w)
		img->x = (win->w - w) / 2;
	else if (img->x > 0)
		img->x = 0;
	else if (img->x + w < win->w)
		img->x = win->w - w;
	if (h < win->h)
		img->y = (win->h - h) / 2;
	else if (img->y > 0)
		img->y = 0;
	else if (img->y + h < win->h)
		img->y = win->h - h;

	if (!moved && (ox != img->x || oy != img->y))
		img->dirty = true;
}

static bool img_fit(img_t *img)
{
	float z, zw, zh;

	if (img->scalemode == SCALE_ZOOM)
		return false;

	zw = (float)img->win->w / (float)img->w;
	zh = (float)img->win->h / (float)img->h;

	switch (img->scalemode) {
	case SCALE_FILL:
		z = MAX(zw, zh);
		break;
	case SCALE_WIDTH:
		z = zw;
		break;
	case SCALE_HEIGHT:
		z = zh;
		break;
	default:
		z = MIN(zw, zh);
		break;
	}
	z = MIN(z, img->scalemode == SCALE_DOWN ? 1.0 : ZOOM_MAX);

	if (ABS(img->zoom - z) > 1.0 / MAX(img->w, img->h)) {
		img->zoom = z;
		img->dirty = title_dirty = true;
		return true;
	} else {
		return false;
	}
}

void img_render(img_t *img)
{
	win_t *win;
	int sx, sy, sw, sh;
	int dx, dy, dw, dh;
	Imlib_Image bg;

	win = img->win;
	img_fit(img);

	if (img->checkpan) {
		img_check_pan(img, false);
		img->checkpan = false;
	}

	if (!img->dirty)
		return;

	/* calculate source and destination offsets:
	 *   - part of image drawn on full window, or
	 *   - full image drawn on part of window
	 */
	if (img->x <= 0) {
		sx = -img->x / img->zoom + 0.5;
		sw = win->w / img->zoom;
		dx = 0;
		dw = win->w;
	} else {
		sx = 0;
		sw = img->w;
		dx = img->x;
		dw = MAX(img->w * img->zoom, 1);
	}
	if (img->y <= 0) {
		sy = -img->y / img->zoom + 0.5;
		sh = win->h / img->zoom;
		dy = win->bar.top ? win->bar.h : 0;
		dh = win->h;
	} else {
		sy = 0;
		sh = img->h;
		dy = img->y + (win->bar.top ? win->bar.h : 0);
		dh = MAX(img->h * img->zoom, 1);
	}

	win_clear(win);

	imlib_context_set_image(img->im);
	imlib_context_set_anti_alias(img->anti_alias);
	imlib_context_set_drawable(win->buf.pm);

	/* manual blending, for performance reasons.
	 * see https://phab.enlightenment.org/T8969#156167 for more details.
	 */
	if (imlib_image_has_alpha()) {
		if ((bg = imlib_create_image(dw, dh)) == NULL) {
			error(0, ENOMEM, "Failed to create image");
			goto fallback;
		}
		imlib_context_set_image(bg);
		imlib_image_set_has_alpha(0);

		if (img->alpha_layer) {
			int i, c, r;
			uint32_t col[2] = { 0xFF666666, 0xFF999999 };
			uint32_t *data = imlib_image_get_data();

			for (r = 0; r < dh; r++) {
				i = r * dw;
				if (r == 0 || r == 8) {
					for (c = 0; c < dw; c++)
						data[i++] = col[!(c & 8) ^ !r];
				} else {
					memcpy(&data[i], &data[(r & 8) * dw], dw * sizeof(data[0]));
				}
			}
			imlib_image_put_back_data(data);
		} else {
			XColor c = win->win_bg;
			imlib_context_set_color(c.red >> 8, c.green >> 8, c.blue >> 8, 0xFF);
			imlib_image_fill_rectangle(0, 0, dw, dh);
		}
		imlib_context_set_blend(1);
		imlib_context_set_operation(IMLIB_OP_COPY);
		imlib_blend_image_onto_image(img->im, 0, sx, sy, sw, sh, 0, 0, dw, dh);
		imlib_context_set_color_modifier(NULL);
		imlib_render_image_on_drawable(dx, dy);
		imlib_free_image();
		imlib_context_set_color_modifier(img->cmod);
	} else {
fallback:
		imlib_render_image_part_on_drawable_at_size(sx, sy, sw, sh, dx, dy, dw, dh);
	}
	img->dirty = false;
}

bool img_fit_win(img_t *img, scalemode_t sm)
{
	float oz;

	oz = img->zoom;
	img->scalemode = sm;

	if (img_fit(img)) {
		img->x = img->win->w / 2 - (img->win->w / 2 - img->x) * img->zoom / oz;
		img->y = img->win->h / 2 - (img->win->h / 2 - img->y) * img->zoom / oz;
		img->checkpan = true;
		return true;
	} else {
		return false;
	}
}

bool img_zoom_to(img_t *img, float z)
{
	int x, y;
	if (ZOOM_MIN <= z && z <= ZOOM_MAX) {
		win_cursor_pos(img->win, &x, &y);
		if (x < 0 || (unsigned int)x >= img->win->w ||
		    y < 0 || (unsigned int)y >= img->win->h)
		{
			x = img->win->w / 2;
			y = img->win->h / 2;
		}
		img->x = x - (x - img->x) * z / img->zoom;
		img->y = y - (y - img->y) * z / img->zoom;
		img->zoom = z;
		img->scalemode = SCALE_ZOOM;
		img->dirty = img->checkpan = title_dirty = true;
		return true;
	} else {
		return false;
	}
}

bool img_zoom(img_t *img, int d)
{
	int i = d > 0 ? 0 : (int)ARRLEN(zoom_levels) - 1;
	while (i >= 0 && i < (int)ARRLEN(zoom_levels) &&
	       (d > 0 ? zoom_levels[i] / 100 <= img->zoom : zoom_levels[i] / 100 >= img->zoom))
	{
		i += d;
	}
	i = MIN(MAX(i, 0), (int)ARRLEN(zoom_levels) - 1);
	return img_zoom_to(img, zoom_levels[i] / 100);
}

bool img_pos(img_t *img, float x, float y)
{
	float ox, oy;

	ox = img->x;
	oy = img->y;

	img->x = x;
	img->y = y;

	img_check_pan(img, true);

	if (ox != img->x || oy != img->y) {
		img->dirty = true;
		return true;
	} else {
		return false;
	}
}

static bool img_move(img_t *img, float dx, float dy)
{
	return img_pos(img, img->x + dx, img->y + dy);
}

bool img_pan(img_t *img, direction_t dir, int d)
{
	/* d < 0: screen-wise
	 * d = 0: 1/PAN_FRACTION of screen
	 * d > 0: num of pixels
	 */
	float x, y;

	if (d > 0) {
		x = y = MAX(1, (float)d * img->zoom);
	} else {
		x = img->win->w / (d < 0 ? 1 : PAN_FRACTION);
		y = img->win->h / (d < 0 ? 1 : PAN_FRACTION);
	}

	switch (dir) {
	case DIR_LEFT:
		return img_move(img, x, 0.0);
	case DIR_RIGHT:
		return img_move(img, -x, 0.0);
	case DIR_UP:
		return img_move(img, 0.0, y);
	case DIR_DOWN:
		return img_move(img, 0.0, -y);
	}
	return false;
}

bool img_pan_center(img_t *img)
{
	float x, y;
	x = (img->win->w - img->w * img->zoom) / 2.0;
	y = (img->win->h - img->h * img->zoom) / 2.0;
	return img_pos(img, x, y);
}

bool img_pan_edge(img_t *img, direction_t dir)
{
	float ox, oy;

	ox = img->x;
	oy = img->y;

	if (dir & DIR_LEFT)
		img->x = 0;
	if (dir & DIR_RIGHT)
		img->x = img->win->w - img->w * img->zoom;
	if (dir & DIR_UP)
		img->y = 0;
	if (dir & DIR_DOWN)
		img->y = img->win->h - img->h * img->zoom;

	img_check_pan(img, true);

	if (ox != img->x || oy != img->y) {
		img->dirty = true;
		return true;
	} else {
		return false;
	}
}

void img_rotate(img_t *img, degree_t d)
{
	unsigned int i, tmp;
	float ox, oy;

	imlib_context_set_image(img->im);
	imlib_image_orientate(d);

	for (i = 0; i < img->multi.cnt; i++) {
		if (i != img->multi.sel) {
			imlib_context_set_image(img->multi.frames[i].im);
			imlib_image_orientate(d);
		}
	}
	if (d == DEGREE_90 || d == DEGREE_270) {
		ox = d == DEGREE_90  ? img->x : img->win->w - img->x - img->w * img->zoom;
		oy = d == DEGREE_270 ? img->y : img->win->h - img->y - img->h * img->zoom;

		img->x = oy + (img->win->w - img->win->h) / 2;
		img->y = ox + (img->win->h - img->win->w) / 2;

		tmp = img->w;
		img->w = img->h;
		img->h = tmp;
		img->checkpan = true;
	}
	img->dirty = true;
}

void img_flip(img_t *img, flipdir_t d)
{
	unsigned int i;
	void (*imlib_flip_op[3])(void) = {
		imlib_image_flip_horizontal,
		imlib_image_flip_vertical,
		imlib_image_flip_diagonal
	};

	d = (d & (FLIP_HORIZONTAL | FLIP_VERTICAL)) - 1;

	if (d < 0 || d >= ARRLEN(imlib_flip_op))
		return;

	imlib_context_set_image(img->im);
	imlib_flip_op[d]();

	for (i = 0; i < img->multi.cnt; i++) {
		if (i != img->multi.sel) {
			imlib_context_set_image(img->multi.frames[i].im);
			imlib_flip_op[d]();
		}
	}
	img->dirty = true;
}

void img_toggle_antialias(img_t *img)
{
	img->anti_alias = !img->anti_alias;
	imlib_context_set_image(img->im);
	imlib_context_set_anti_alias(img->anti_alias);
	img->dirty = true;
}

static double steps_to_range(int d, double max, double offset)
{
	return offset + d * ((d <= 0 ? 1.0 : (max - 1.0)) / CC_STEPS);
}

void img_update_color_modifiers(img_t *img)
{
	assert(imlib_context_get_color_modifier() == img->cmod);
	imlib_reset_color_modifier();

	if (img->gamma != 0)
		imlib_modify_color_modifier_gamma(steps_to_range(img->gamma, GAMMA_MAX, 1.0));
	if (img->brightness != 0)
		imlib_modify_color_modifier_brightness(steps_to_range(img->brightness, BRIGHTNESS_MAX, 0.0));
	if (img->contrast != 0)
		imlib_modify_color_modifier_contrast(steps_to_range(img->contrast, CONTRAST_MAX, 1.0));

	img->dirty = true;
}

bool img_change_color_modifier(img_t *img, int d, int *target)
{
	int value = d == 0 ? 0 : MIN(MAX(*target + d, -CC_STEPS), CC_STEPS);

	if (*target == value)
		return false;

	*target = value;
	img_update_color_modifiers(img);
	return true;
}

static bool img_frame_goto(img_t *img, int n)
{
	if (n < 0 || (unsigned int)n >= img->multi.cnt || (unsigned int)n == img->multi.sel)
		return false;

	img->multi.sel = n;
	img->im = img->multi.frames[n].im;

	imlib_context_set_image(img->im);
	img->w = imlib_image_get_width();
	img->h = imlib_image_get_height();
	img->checkpan = true;
	img->dirty = true;

	return true;
}

bool img_frame_navigate(img_t *img, int d)
{
	if (img->multi.cnt == 0 || d == 0)
		return false;

	d += img->multi.sel;
	d = MAX(0, MIN(d, (int)img->multi.cnt - 1));

	return img_frame_goto(img, d);
}

bool img_frame_animate(img_t *img)
{
	if (img->multi.cnt > 0)
		return img_frame_goto(img, (img->multi.sel + 1) % img->multi.cnt);
	else
		return false;
}
