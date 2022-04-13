# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>

void sys_error(char * str)
{
    perror(str);
    exit(-1);
}

int main(void)
{
    int pipe_fd[2];
    if ( pipe(pipe_fd) == -1)
    {
        sys_error("pipe fails");
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        sys_error("fork fails");
    }
    else if (pid > 0)
    {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        execlp("ls", "ls", "-l", NULL);
    }
    else
    {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        execlp("wc", "wc", "-l", NULL);
    }
    return 0;
}
