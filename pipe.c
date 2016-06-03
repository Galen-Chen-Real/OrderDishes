/*
 *        'pipe.c'
 *     (C) 2014.4 GordonChen
 *
 */
#include "pipe.h"

static PIPEFD fd;

int pipe_init(void)
{
    if(pipe(fd))
    {
        perror("pipe() failed");
        return -1;
    }
    return 0;
}

int pipe_send(const void *buf, int len)
{
    return write(fd[1], buf, len);
}

int pipe_recv(void *buf, int len)
{
    return read(fd[0], buf, len);
}

void pipe_close(void)
{
    close(fd[0]);
    close(fd[1]);
}

