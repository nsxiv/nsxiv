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

char *r_readdir(r_dir_t *rdir, bool skip_dotfiles)
{
	size_t len;
	char *filename;
	struct dirent *dentry;
	struct stat fstats;

	while (true) {
		if (rdir->dir != NULL && (dentry = readdir(rdir->dir)) != NULL) {
			if (dentry->d_name[0] == '.') {
				if (skip_dotfiles)
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

static int mkspawn_pipe(posix_spawn_file_actions_t *fa, const char *cmd, int *pfd, int dupidx)
{
	int err;
	if (pipe(pfd) < 0) {
		error(0, errno, "pipe: %s", cmd);
		return -1;
	}
	err = posix_spawn_file_actions_adddup2(fa, pfd[dupidx], dupidx);
	err = err ? err : posix_spawn_file_actions_addclose(fa, pfd[0]);
	err = err ? err : posix_spawn_file_actions_addclose(fa, pfd[1]);
	if (err) {
		error(0, err, "posix_spawn_file_actions: %s", cmd);
		close(pfd[0]);
		close(pfd[1]);
	}
	return err ? -1 : 0;
}

pid_t spawn(int *readfd, int *writefd, char *const argv[])
{
	pid_t pid = -1;
	const char *cmd;
	int err, pfd_read[2], pfd_write[2];
	posix_spawn_file_actions_t fa;

	assert(argv != NULL && argv[0] != NULL);
	cmd = argv[0];

	if ((err = posix_spawn_file_actions_init(&fa)) != 0) {
		error(0, err, "spawn: %s", cmd);
		return pid;
	}

	if (readfd != NULL && mkspawn_pipe(&fa, cmd, pfd_read, 1) < 0)
		goto err_destroy_fa;
	if (writefd != NULL && mkspawn_pipe(&fa, cmd, pfd_write, 0) < 0)
		goto err_close_readfd;

	if ((err = posix_spawnp(&pid, cmd, &fa, NULL, argv, environ)) != 0) {
		error(0, err, "spawn: %s", cmd);
	} else {
		if (readfd != NULL)
			*readfd = pfd_read[0];
		if (writefd != NULL)
			*writefd = pfd_write[1];
	}

	if (writefd != NULL) {
		close(pfd_write[0]);
		if (pid < 0)
			close(pfd_write[1]);
	}
err_close_readfd:
	if (readfd != NULL) {
		if (pid < 0)
			close(pfd_read[0]);
		close(pfd_read[1]);
	}
err_destroy_fa:
	posix_spawn_file_actions_destroy(&fa);
	return pid;
}
