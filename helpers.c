#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "helpers.h"
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

void spawn(char *cmd[])
{
    if (fork() == 0) {
        if (dpy != NULL)
            close(xcb_get_file_descriptor(dpy));
        if (fork() == 0) {
            setsid();
            if (redir_fd != -1) {
                dup2(redir_fd, STDOUT_FILENO);
                dup2(redir_fd, STDERR_FILENO);
            }
            execvp(cmd[0], cmd);
            err("Spawning failed.\n");
        }
        exit(EXIT_SUCCESS);
    }
    wait(NULL);
}

void run(char *command)
{
    char *cmd[] = {shell, "-c", command, NULL};
    spawn(cmd);
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
