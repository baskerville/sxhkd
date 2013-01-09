#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
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
