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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern char **environ;
const char *progname = "nsxiv";

void *emalloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (ptr == NULL)
		error(EXIT_FAILURE, errno, NULL);
	return ptr;
}

void *ecalloc(size_t nmemb, size_t size)
{
	void *ptr;

	ptr = calloc(nmemb, size);
	if (ptr == NULL)
		error(EXIT_FAILURE, errno, NULL);
	return ptr;
}

void *erealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL)
		error(EXIT_FAILURE, errno, NULL);
	return ptr;
}

char *estrdup(const char *s)
{
	size_t n = strlen(s) + 1;
	return memcpy(emalloc(n), s, n);
}

char *estrndup(const char *s, size_t n)
{
	char *ptr = strndup(s, n);
	if (ptr == NULL)
		error(EXIT_FAILURE, errno, NULL);
	return ptr;
}

void error(int eval, int err, const char *fmt, ...)
{
	va_list ap;

	if (eval == 0 && options->quiet)
		return;

	fflush(stdout);
	fprintf(stderr, "%s: ", progname);
	va_start(ap, fmt);
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (err != 0)
		fprintf(stderr, "%s%s", fmt != NULL ? ": " : "", strerror(err));
	fputc('\n', stderr);

	if (eval != 0)
		exit(eval);
}

const char *file_realpath(const fileinfo_t *file, bool invalidate_symlinks)
{
	bool use_cached, need_refresh = false;
	fileinfo_t *mutable = (fileinfo_t *)file; /* avoids cast on caller's side */

	assert(file != NULL);

	use_cached = file->path != NULL && !invalidate_symlinks;
	if (!use_cached) {
		struct stat lst;
		bool was_symlink = file->flags & FF_SYMLINK;
		if (lstat(file->name, &lst) == 0 && S_ISLNK(lst.st_mode))
			mutable->flags |= FF_SYMLINK;
		else
			mutable->flags &= ~FF_SYMLINK;
		need_refresh = file->path == NULL ||
		               (invalidate_symlinks && (was_symlink || (file->flags & FF_SYMLINK)));
	}
	if (need_refresh) {
		char *newpath = realpath(file->name, NULL);
		if (newpath == NULL) {
			if (file->flags & FF_WARN)
				error(0, errno, "%s", file->name);
		}
		if (newpath == NULL || file->path == NULL || !STREQ(newpath, file->path)) {
			if (invalidate_symlinks && file->path != NULL) {
				mutable->flags |= FF_TN_NEEDS_UPDATE;
			}
			free((void *)mutable->path);
			mutable->path = newpath;
		} else {
			free(newpath);
		}
	}
	return file->path;
}

int r_opendir(r_dir_t *rdir, const char *dirname, bool recursive)
{
	if (*dirname == '\0')
		return -1;

	if ((rdir->dir = opendir(dirname)) == NULL) {
		rdir->name = NULL;
		rdir->stack = NULL;
		return -1;
	}

	rdir->stcap = 512;
	rdir->stack = emalloc(rdir->stcap * sizeof(*rdir->stack));
	rdir->stlen = 0;

	rdir->name = (char *)dirname;
	rdir->d = 0;
	rdir->recursive = recursive;

	return 0;
}

int r_closedir(r_dir_t *rdir)
{
	int ret = 0;

	if (rdir->stack != NULL) {
		while (rdir->stlen > 0)
			free(rdir->stack[--rdir->stlen]);
		free(rdir->stack);
		rdir->stack = NULL;
	}

	if (rdir->dir != NULL) {
		if ((ret = closedir(rdir->dir)) == 0)
			rdir->dir = NULL;
	}

	if (rdir->d != 0) {
		free(rdir->name);
		rdir->name = NULL;
	}

	return ret;
}

char *r_readdir(r_dir_t *rdir, bool include_hidden)
{
	size_t len;
	char *filename;
	struct dirent *dentry;
	struct stat fstats;

	while (true) {
		if (rdir->dir != NULL && (dentry = readdir(rdir->dir)) != NULL) {
			if (dentry->d_name[0] == '.') {
				if (!include_hidden)
					continue;
				if (dentry->d_name[1] == '\0')
					continue;
				if (dentry->d_name[1] == '.' && dentry->d_name[2] == '\0')
					continue;
			}

			len = strlen(rdir->name) + strlen(dentry->d_name) + 2;
			filename = emalloc(len);
			snprintf(filename, len, "%s%s%s", rdir->name,
			         rdir->name[strlen(rdir->name) - 1] == '/' ? "" : "/",
			         dentry->d_name);

			if (stat(filename, &fstats) < 0) {
				free(filename);
				continue;
			}
			if (S_ISDIR(fstats.st_mode)) {
				/* put subdirectory on the stack */
				if (rdir->stlen == rdir->stcap) {
					rdir->stcap *= 2;
					rdir->stack = erealloc(rdir->stack,
					                       rdir->stcap * sizeof(*rdir->stack));
				}
				rdir->stack[rdir->stlen++] = filename;
				continue;
			}
			return filename;
		}

		if (rdir->recursive && rdir->stlen > 0) {
			/* open next subdirectory */
			assert(rdir->dir != NULL);
			closedir(rdir->dir);
			if (rdir->d != 0)
				free(rdir->name);
			rdir->name = rdir->stack[--rdir->stlen];
			rdir->d = 1;
			if ((rdir->dir = opendir(rdir->name)) == NULL)
				error(0, errno, "%s", rdir->name);
			continue;
		}
		/* no more entries */
		break;
	}
	return NULL;
}

int r_mkdir(char *path)
{
	int rc = 0;
	char c, *s = path;
	struct stat st;

	while (*s != '\0' && rc == 0) {
		if (*s == '/') {
			s++;
			continue;
		}
		for (; *s != '\0' && *s != '/'; s++)
			;
		c = *s;
		*s = '\0';
		if (mkdir(path, 0755) == -1) {
			if (errno != EEXIST || stat(path, &st) == -1 || !S_ISDIR(st.st_mode)) {
				error(0, errno, "%s", path);
				rc = -1;
			}
		}
		*s = c;
	}
	return rc;
}

void construct_argv(char **argv, unsigned int len, ...)
{
	unsigned int i;
	va_list args;

	va_start(args, len);
	for (i = 0; i < len; ++i)
		argv[i] = va_arg(args, char *);
	va_end(args);
	assert(argv[len - 1] == NULL && "argv should be NULL terminated");
}

static int mkspawn_pipe(posix_spawn_file_actions_t *fa, const char *cmd, int *pfd, int dupidx, int pipeflags)
{
	int err = 0;
	if (pipe(pfd) < 0)
		return errno;
	if (pipeflags && (fcntl(pfd[0], F_SETFL, pipeflags) < 0 || fcntl(pfd[1], F_SETFL, pipeflags) < 0))
		err = errno;
	err = err ? err : posix_spawn_file_actions_adddup2(fa, pfd[dupidx], dupidx);
	err = err ? err : posix_spawn_file_actions_addclose(fa, pfd[0]);
	err = err ? err : posix_spawn_file_actions_addclose(fa, pfd[1]);
	return err;
}

pid_t spawn(int *readfd, int *writefd, int pipeflags, char *const argv[])
{
	pid_t pid = -1;
	const char *cmd;
	int pfd_read[2] = {-1, -1}, pfd_write[2] = {-1, -1};
	int err = 0;
	bool fa_initialized = false, attr_initialized = false;
	posix_spawn_file_actions_t fa;
	sigset_t sigset;
	posix_spawnattr_t attr;

	assert(argv != NULL && argv[0] != NULL);
	cmd = argv[0];

	fa_initialized = !err && (err = posix_spawn_file_actions_init(&fa)) == 0;
	if (!err && !(err = posix_spawnattr_init(&attr))) {
		attr_initialized = true;
		sigfillset(&sigset);
		err = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGDEF);
		err = err ? err : posix_spawnattr_setsigdefault(&attr, &sigset);
	}

	if (!err && readfd != NULL)
		err = mkspawn_pipe(&fa, cmd, pfd_read, 1, pipeflags);
	if (!err && writefd != NULL)
		err = mkspawn_pipe(&fa, cmd, pfd_write, 0, pipeflags);

	err = err ? err : posix_spawnp(&pid, cmd, &fa, &attr, argv, environ);

	if (pfd_read[0] >= 0) {
		*readfd = err ? (close(pfd_read[0]), -1) : pfd_read[0];
		close(pfd_read[1]);
	}
	if (pfd_write[0] >= 0) {
		close(pfd_write[0]);
		*writefd = err ? (close(pfd_write[1]), -1) : pfd_write[1];
	}
	if (attr_initialized)
		posix_spawnattr_destroy(&attr);
	if (fa_initialized)
		posix_spawn_file_actions_destroy(&fa);

	if (err)
		error(0, err, "spawn: %s", cmd);
	return err ? -1 : pid;
}
