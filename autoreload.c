/* Copyright 2017 Max Voit, Bert Muennich
 * Copyright 2022-2023 nsxiv contributors
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

#if HAVE_INOTIFY

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

void arl_init(arl_t *arl)
{
	arl->fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
	arl->wd_dir = arl->wd_file = arl->wd_link = arl->wd_link_dir = -1;
	arl->filename = arl->linkname = NULL;
	if (arl->fd == -1)
		error(0, 0, "Could not initialize inotify, no automatic image reloading");
}

CLEANUP void arl_cleanup(arl_t *arl)
{
	if (arl->fd != -1)
		close(arl->fd);
	free(arl->filename);
	free(arl->linkname);
}

static void rm_watch(int fd, int *wd)
{
	if (*wd != -1) {
		inotify_rm_watch(fd, *wd);
		*wd = -1;
	}
}

static void add_watch(int fd, int *wd, const char *path, uint32_t mask)
{
	*wd = inotify_add_watch(fd, path, mask);
	if (*wd == -1)
		error(0, errno, "inotify: %s", path);
}

static char *arl_watch_basedir(int fd, int *wd_dir_fd, const char *path)
{
	char dot[] = ".", *base = strrchr(path, '/');
	char *dir = base != NULL ? estrndup(path, MAX(base - path, 1)) : dot;
	add_watch(fd, wd_dir_fd, dir, IN_CREATE | IN_MOVED_TO);
	if (dir != dot)
		free(dir);
	return estrdup(base != NULL ? base + 1 : path);
}

void arl_add(arl_t *arl, const fileinfo_t *file)
{
	if (arl->fd == -1)
		return;

	rm_watch(arl->fd, &arl->wd_dir);
	rm_watch(arl->fd, &arl->wd_file);
	rm_watch(arl->fd, &arl->wd_link);
	rm_watch(arl->fd, &arl->wd_link_dir);
	free(arl->filename);
	arl->filename = NULL;
	free(arl->linkname);
	arl->linkname = NULL;

	add_watch(arl->fd, &arl->wd_file, file->path, IN_CLOSE_WRITE | IN_DELETE_SELF);
	arl->filename = arl_watch_basedir(arl->fd, &arl->wd_dir, file->path);
	if (file->flags & FF_SYMLINK) {
		add_watch(arl->fd, &arl->wd_link, file->name,
		          IN_DELETE_SELF | IN_MOVE_SELF | IN_DONT_FOLLOW);
		arl->linkname = arl_watch_basedir(arl->fd, &arl->wd_link_dir, file->name);
	}
}

bool arl_handle(arl_t *arl)
{
	bool reload = false;
	char *ptr;
	const struct inotify_event *e;
	/* inotify_event aligned buffer */
	static union {
		char d[4096];
		struct inotify_event e;
	} buf;

	while (true) {
		ssize_t len = read(arl->fd, buf.d, sizeof(buf.d));

		if (len == -1) {
			if (errno == EINTR)
				continue;
			break;
		}
		for (ptr = buf.d; ptr < buf.d + len; ptr += sizeof(*e) + e->len) {
			e = (const struct inotify_event *)ptr;
			if (e->wd == arl->wd_file && (e->mask & IN_CLOSE_WRITE)) {
				reload = true;
			} else if (e->wd == arl->wd_file && (e->mask & IN_DELETE_SELF)) {
				rm_watch(arl->fd, &arl->wd_file);
			} else if (e->wd == arl->wd_link && (e->mask & (IN_DELETE_SELF | IN_MOVE_SELF))) {
				rm_watch(arl->fd, &arl->wd_link);
			} else if (e->wd == arl->wd_link_dir && (e->mask & (IN_CREATE | IN_MOVED_TO))) {
				if (STREQ(e->name, arl->linkname))
					reload = true;
			} else if (e->wd == arl->wd_dir && (e->mask & (IN_CREATE | IN_MOVED_TO))) {
				if (STREQ(e->name, arl->filename))
					reload = true;
			}
		}
	}
	return reload;
}

#else

void arl_init(arl_t *arl)
{
	arl->fd = -1;
}

void arl_cleanup(arl_t *arl)
{
	(void)arl;
}

void arl_add(arl_t *arl, const fileinfo_t *file)
{
	(void)arl;
	(void)file;
}

bool arl_handle(arl_t *arl)
{
	(void)arl;
	return false;
}

#endif /* HAVE_INOTIFY */
