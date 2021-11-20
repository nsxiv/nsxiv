/* Copyright 2011-2020 Bert Muennich
 * Copyright 2021 nsxiv contributors
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
#define _TITLE_CONFIG
#include "config.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const opt_t *options;

void print_usage(void)
{
	printf("usage: nsxiv [-abcfhiopqrtvZ0] [-A FRAMERATE] [-e WID] [-G GAMMA] "
	       "[-g GEOMETRY] [-N NAME] [-T TITLE] [-n NUM] [-S DELAY] [-s MODE] "
	       "[-z ZOOM] FILES...\n");
}

static void print_version(void)
{
	puts("nsxiv " VERSION);
}

void parse_options(int argc, char **argv)
{
	int n, opt;
	char *end, *s;
	const char *scalemodes = "dfFwh";
	static opt_t _options;
	options = &_options;

	progname = strrchr(argv[0], '/');
	progname = progname ? progname + 1 : argv[0];

	_options.from_stdin = false;
	_options.to_stdout = false;
	_options.using_null = false;
	_options.recursive = false;
	_options.startnum = 0;

	_options.scalemode = SCALE_DOWN;
	_options.zoom = 1.0;
	_options.animate = false;
	_options.gamma = 0;
	_options.slideshow = 0;
	_options.framerate = 0;

	_options.fullscreen = false;
	_options.embed = 0;
	_options.hide_bar = false;
	_options.geometry = NULL;
	_options.res_name = NULL;
	_options.title_prefix = TITLE_PREFIX;
	_options.title_suffixmode = TITLE_SUFFIXMODE;

	_options.quiet = false;
	_options.thumb_mode = false;
	_options.clean_cache = false;
	_options.private_mode = false;

	while ((opt = getopt(argc, argv, "A:abce:fG:g:hin:N:opqrS:s:T:tvZz:0")) != -1) {
		switch (opt) {
			case '?':
				print_usage();
				exit(EXIT_FAILURE);
			case 'A':
				n = strtol(optarg, &end, 0);
				if (*end != '\0' || n <= 0)
					error(EXIT_FAILURE, 0, "Invalid argument for option -A: %s", optarg);
				_options.framerate = n;
				/* fall through */
			case 'a':
				_options.animate = true;
				break;
			case 'b':
				_options.hide_bar = true;
				break;
			case 'c':
				_options.clean_cache = true;
				break;
			case 'e':
				n = strtol(optarg, &end, 0);
				if (*end != '\0')
					error(EXIT_FAILURE, 0, "Invalid argument for option -e: %s", optarg);
				_options.embed = n;
				break;
			case 'f':
				_options.fullscreen = true;
				break;
			case 'G':
				n = strtol(optarg, &end, 0);
				if (*end != '\0')
					error(EXIT_FAILURE, 0, "Invalid argument for option -G: %s", optarg);
				_options.gamma = n;
				break;
			case 'g':
				_options.geometry = optarg;
				break;
			case 'h':
				print_usage();
				exit(EXIT_SUCCESS);
			case 'i':
				_options.from_stdin = true;
				break;
			case 'n':
				n = strtol(optarg, &end, 0);
				if (*end != '\0' || n <= 0)
					error(EXIT_FAILURE, 0, "Invalid argument for option -n: %s", optarg);
				_options.startnum = n - 1;
				break;
			case 'N':
				_options.res_name = optarg;
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
				n = strtof(optarg, &end) * 10;
				if (*end != '\0' || n <= 0)
					error(EXIT_FAILURE, 0, "Invalid argument for option -S: %s", optarg);
				_options.slideshow = n;
				break;
			case 's':
				s = strchr(scalemodes, optarg[0]);
				if (s == NULL || *s == '\0' || strlen(optarg) != 1)
					error(EXIT_FAILURE, 0, "Invalid argument for option -s: %s", optarg);
				_options.scalemode = s - scalemodes;
				break;
			case 'T':
				if ((s = strrchr(optarg, ':')) != NULL) {
					*s = '\0';
					n = strtol(++s, &end, 0);
					if (*end != '\0' || n < SUFFIX_EMPTY || n > SUFFIX_FULLPATH)
						error(EXIT_FAILURE, 0, "Invalid argument for option -T suffixmode: %s", s);
					_options.title_suffixmode = n;
				}
				_options.title_prefix = optarg;
				break;
			case 't':
				_options.thumb_mode = true;
				break;
			case 'v':
				print_version();
				exit(EXIT_SUCCESS);
			case 'Z':
				_options.scalemode = SCALE_ZOOM;
				_options.zoom = 1.0;
				break;
			case 'z':
				n = strtol(optarg, &end, 0);
				if (*end != '\0' || n <= 0)
					error(EXIT_FAILURE, 0, "Invalid argument for option -z: %s", optarg);
				_options.scalemode = SCALE_ZOOM;
				_options.zoom = (float) n / 100.0;
				break;
			case '0':
				_options.using_null = true;
				break;
		}
	}

	_options.filenames = argv + optind;
	_options.filecnt = argc - optind;

	if (_options.filecnt == 1 && STREQ(_options.filenames[0], "-")) {
		_options.filenames++;
		_options.filecnt--;
		_options.from_stdin = true;
	}
}
