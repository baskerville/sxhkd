/* Copyright (c) 2013, Bastien Dejean
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sxhkd.h"

void warn(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

__attribute__((noreturn))
void err(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void run(char *command, bool sync)
{
	char *cmd[] = {shell, "-c", command, NULL};
	spawn(cmd, sync);
}

void spawn(char *cmd[], bool sync)
{
	if (fork() == 0) {
		if (dpy != NULL)
			close(xcb_get_file_descriptor(dpy));
		if (sync) {
			execute(cmd);
		} else {
			if (fork() == 0) {
				execute(cmd);
			}
			exit(EXIT_SUCCESS);
		}
	}
	wait(NULL);
}

void execute(char *cmd[])
{
	setsid();
	if (redir_fd != -1) {
		dup2(redir_fd, STDOUT_FILENO);
		dup2(redir_fd, STDERR_FILENO);
	}
	execvp(cmd[0], cmd);
	err("Spawning failed.\n");
}

char *lgraph(char *s)
{
	size_t len = strlen(s);
	unsigned int i = 0;
	while (i < len && !isgraph(s[i]))
		i++;
	if (i < len)
		return (s + i);
	else
		return NULL;
}

char *rgraph(char *s)
{
	int i = strlen(s) - 1;
	while (i >= 0 && !isgraph(s[i]))
		i--;
	if (i >= 0)
		return (s + i);
	else
		return NULL;
}
