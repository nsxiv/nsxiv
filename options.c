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
#include "version.h"
#define INCLUDE_OPTIONS_CONFIG
#include "config.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#pragma GCC diagnostic push /* also works on clang */
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-function"
#include "optparse.h"
#pragma GCC diagnostic pop

const opt_t *options;

void print_usage(FILE *stream)
{
	fprintf(stream,
	        "usage: %s [-abcfhiopqrtvZ0] [-A FRAMERATE] [-e WID] [-G GAMMA] "
	        "[-g GEOMETRY] [-N NAME] [-n NUM] [-S DELAY] [-s MODE] "
	        "[-z ZOOM] FILES...\n",
	        progname);
}

static void print_version(void)
{
	printf("%s %s\n", progname, VERSION);
	fputs("features: "
#if HAVE_INOTIFY
		"+inotify "
#endif
#if HAVE_LIBFONTS
		"+statusbar "
#endif
#if HAVE_LIBEXIF
		"+exif "
#endif
		"+multiframe "
		"\n", stdout);
}

static bool parse_optional_no(const char *flag, const char *arg)
{
	if (arg != NULL && !STREQ(arg, "no"))
		error(EXIT_FAILURE, 0, "Invalid argument for option --%s: %s", flag, arg);
	return arg == NULL;
}

void parse_options(int argc, char **argv)
{
	enum {
		/* ensure these can't be represented in a single byte in order
		 * to avoid conflicts with short opts
		 */
		OPT_START = UCHAR_MAX,
		OPT_AA,
		OPT_AL,
		OPT_THUMB,
		OPT_BAR,
		OPT_CLASS,
		OPT_CA,
		OPT_CD,
		OPT_UC
	};
	static const struct optparse_long longopts[] = {
		{ "framerate",      'A',     OPTPARSE_REQUIRED },
		{ "animate",        'a',     OPTPARSE_NONE },
		{ "no-bar",         'b',     OPTPARSE_NONE },
		{ "bar",          OPT_BAR,   OPTPARSE_NONE },
		{ "clean-cache",    'c',     OPTPARSE_NONE },
		{ "embed",          'e',     OPTPARSE_REQUIRED },
		{ "fullscreen",     'f',     OPTPARSE_NONE },
		{ "gamma",          'G',     OPTPARSE_REQUIRED },
		{ "geometry",       'g',     OPTPARSE_REQUIRED },
		{ "help",           'h',     OPTPARSE_NONE },
		{ "stdin",          'i',     OPTPARSE_NONE },
		{ "name",           'N',     OPTPARSE_REQUIRED },
		{ "class",       OPT_CLASS,  OPTPARSE_REQUIRED },
		{ "start-at",       'n',     OPTPARSE_REQUIRED },
		{ "stdout",         'o',     OPTPARSE_NONE },
		{ "private",        'p',     OPTPARSE_NONE },
		{ "quiet",          'q',     OPTPARSE_NONE },
		{ "recursive",      'r',     OPTPARSE_NONE },
		{ "ss-delay",       'S',     OPTPARSE_REQUIRED },
		{ "scale-mode",     's',     OPTPARSE_REQUIRED },
		/* short opt `-t` doesn't accept optional arg for backwards compatibility reasons */
		{ NULL,             't',     OPTPARSE_NONE },
		{ "thumbnail",   OPT_THUMB,  OPTPARSE_OPTIONAL },
		{ "version",        'v',     OPTPARSE_NONE },
		{ "zoom-100",       'Z',     OPTPARSE_NONE },
		{ "zoom",           'z',     OPTPARSE_REQUIRED },
		{ "null",           '0',     OPTPARSE_NONE },
		{ "anti-alias",    OPT_AA,   OPTPARSE_OPTIONAL },
		{ "alpha-layer",   OPT_AL,   OPTPARSE_OPTIONAL },
		{ "cache-allow",   OPT_CA,   OPTPARSE_REQUIRED },
		{ "cache-deny",    OPT_CD,   OPTPARSE_REQUIRED },
		{ "update-cache",  OPT_UC,   OPTPARSE_NONE },
		{ 0 }, /* end */
	};

	long n, opt;
	float f;
	char *end, *s;
	struct optparse op;
	const char scalemodes[] = "dfFwh"; /* must be sorted according to scalemode_t */
	static opt_t _options;

	options = &_options;
	_options.from_stdin = false;
	_options.to_stdout = false;
	_options.using_null = false;
	_options.recursive = false;
	_options.startnum = 0;

	_options.scalemode = SCALE_DOWN;
	_options.zoom = 1.0;
	_options.anti_alias = ANTI_ALIAS;
	_options.alpha_layer = ALPHA_LAYER;
	_options.animate = false;
	_options.gamma = 0;
	_options.slideshow = 0;
	_options.framerate = 0;

	_options.fullscreen = false;
	_options.embed = 0;
	_options.hide_bar = false;
	_options.geometry = NULL;
	_options.res_name = NULL;

	_options.tns_filters = TNS_FILTERS;
	_options.tns_filters_is_blacklist = TNS_FILTERS_IS_BLACKLIST;
	_options.quiet = false;
	_options.thumb_mode = false;
	_options.clean_cache = false;
	_options.update_cache = false;
	_options.private_mode = false;

	if (argc > 0) {
		s = strrchr(argv[0], '/');
		progname = s != NULL && s[1] != '\0' ? s + 1 : argv[0];
	}

	optparse_init(&op, argv);
	while ((opt = optparse_long(&op, longopts, NULL)) != -1) {
		for (n = 0; n < (int)ARRLEN(longopts); ++n) { /* clang-tidy finds some non-sensical branch and thinks optarg == NULL is possible */
			if (opt == longopts[n].shortname && longopts[n].argtype == OPTPARSE_REQUIRED)
				assert(op.optarg != NULL);
		}
		switch (opt) {
		case '?':
			fprintf(stderr, "%s\n", op.errmsg);
			print_usage(stderr);
			exit(EXIT_FAILURE);
		case 'A':
			n = strtol(op.optarg, &end, 0);
			if (*end != '\0' || n < 0 || n > INT_MAX)
				error(EXIT_FAILURE, 0, "Invalid framerate: %s", op.optarg);
			_options.framerate = n;
			_options.animate = n > 0;
			break;
		case 'a':
			_options.animate = true;
			break;
		case 'b': case OPT_BAR:
			_options.hide_bar = (opt == 'b');
			break;
		case 'c':
			_options.clean_cache = true;
			break;
		case 'e':
			n = strtol(op.optarg, &end, 0);
			if (*end != '\0')
				error(EXIT_FAILURE, 0, "Invalid window id: %s", op.optarg);
			_options.embed = n;
			break;
		case 'f':
			_options.fullscreen = true;
			break;
		case 'G':
			n = strtol(op.optarg, &end, 0);
			if (*end != '\0' || n < INT_MIN || n > INT_MAX)
				error(EXIT_FAILURE, 0, "Invalid gamma: %s", op.optarg);
			_options.gamma = n;
			break;
		case 'g':
			_options.geometry = op.optarg;
			break;
		case 'h':
			print_usage(stdout);
			exit(EXIT_SUCCESS);
		case 'i':
			_options.from_stdin = true;
			break;
		case 'n':
			n = strtol(op.optarg, &end, 0);
			if (*end != '\0' || n <= 0 || n > INT_MAX)
				error(EXIT_FAILURE, 0, "Invalid starting number: %s", op.optarg);
			_options.startnum = n - 1;
			break;
		case OPT_CLASS:
			error(0, 0, "--class is deprecated, use --name instead");
			/* fallthrough */
		case 'N':
			_options.res_name = op.optarg;
			break;
		case 'o':
			_options.to_stdout = true;
			break;
		case 'p':
			_options.private_mode = true;
			break;
		case 'q':
			_options.quiet = true;
			break;
		case 'r':
			_options.recursive = true;
			break;
		case 'S':
			f = strtof(op.optarg, &end) * 10.0f;
			if (*end != '\0' || f <= 0 || f >= (float)UINT_MAX)
				error(EXIT_FAILURE, 0, "Invalid slideshow delay: %s", op.optarg);
			_options.slideshow = (unsigned int)f;
			break;
		case 's':
			s = strchr(scalemodes, op.optarg[0]);
			if (s == NULL || *s == '\0' || strlen(op.optarg) != 1)
				error(EXIT_FAILURE, 0, "Invalid scale mode: %s", op.optarg);
			_options.scalemode = s - scalemodes;
			break;
		case 't':
			_options.thumb_mode = true;
			break;
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'Z':
			_options.scalemode = SCALE_ZOOM;
			_options.zoom = 1.0f;
			break;
		case 'z':
			n = strtol(op.optarg, &end, 0);
			if (*end != '\0' || n <= 0)
				error(EXIT_FAILURE, 0, "Invalid zoom level: %s", op.optarg);
			_options.scalemode = SCALE_ZOOM;
			_options.zoom = (float)n / 100.0f;
			break;
		case '0':
			_options.using_null = true;
			break;
		case OPT_AA:
			_options.anti_alias = parse_optional_no("anti-alias", op.optarg);
			break;
		case OPT_AL:
			_options.alpha_layer = parse_optional_no("alpha-layer", op.optarg);
			break;
		case OPT_THUMB:
			_options.thumb_mode = parse_optional_no("thumbnail", op.optarg);
			break;
		case OPT_CA: case OPT_CD:
			_options.tns_filters = op.optarg;
			_options.tns_filters_is_blacklist = (opt == OPT_CD);
			break;
		case OPT_UC:
			_options.update_cache = true;
			break;
		}
	}

	_options.filenames = argv + op.optind;
	_options.filecnt = argc - op.optind;

	if (_options.filecnt == 1 && STREQ(_options.filenames[0], "-")) {
		_options.filenames++;
		_options.filecnt--;
		_options.from_stdin = true;
	}
}
