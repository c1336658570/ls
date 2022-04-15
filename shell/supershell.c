/*
实现 管道 (也就是 |)
实现 输入输出重定向(也就是 < > >>)
实现 后台运行（也就是 & ）
实现 cd，要求支持能切换到绝对路径，相对路径和支持 cd -
屏蔽一些信号（如 ctrl + c 不能终止）
*/

# include "supershell.h"

int main(void)
{
    int account, i;
    char *buf;
    char argvlist[50][256];
    sigset_t sig;
    shield_signal(sig);

    while (1)
    {
        ps1();
        buf = readline(NULL);
        add_history(buf);
        if (buf[0] == 0)
            printf("\n");
        if ( !strcmp(buf, "exit") )
        {
            break;
        }
        account = 0;
        for (i = 0; i < 50; ++i)
        {
            argvlist[i][0] = 0;
        }
        command_parsing(buf, argvlist, &account);
        do_cmd(account, argvlist);

        free(buf);
    }

    return 0;
}

void ps1()
{
    char *path = (char *)malloc(256);
    getcwd(path, 256);
    char *name = "cmf-super-shell";

    printf("\001\033[35m\002%s\001\033[0m:\002\001\033[31m\002%s\001\033[0m\002$ ", name, path);

    free(path);
}


void command_parsing(char *buf, char (*arg)[256], int *commandsize)
{
    int number = 0, i;
    int len = strlen(buf);

    for (i = 0; i < len; ++i)
    {
        //解析每个命令，以空格做为分隔
        if (buf[i] != ' ')
        {
            arg[(*commandsize)][number++] = buf[i];
        }
        else
        {
            arg[(*commandsize)][number] = '\0';
            while (buf[i] == ' ' && i < len)
            {
                i++;
            }
            //执行完后buf[i]不为空格，但是for循环最后有个++i，所以i应该减1
            if (i != len)
            {
                i--;
            }
            (*commandsize)++;
            number = 0;
        }
        //若最后一个字符不是空格，不会进入else，命令个数统计会少一个，所以需要手动加1
        if (i == len-1 && buf[i] != ' ')
        {
            arg[(*commandsize)][number] = '\0';
            (*commandsize)++;
        }
    }
}

void do_cmd(int account, char (*arg)[256])
{
    int i;

    //后台运行
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], "&") == 0 )
        {
            background = 1;
        }
    }
    //cd命令
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], "cd") == 0 )
        {
            command_cd(account, arg);
            return;
        }
    }
    //管道
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], "|") == 0 )
        {
            command_pipe3(account, arg);
            return;
        }
    }

    //重定向
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], ">") == 0 || strcmp(arg[i], "<") == 0 || strcmp(arg[i], ">>") == 0 )
        {
            out_in_append(account, arg);
            return;
        }
    }

    /*
    //输出重定向
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], ">") == 0 )
        {
            output_redirect(account, arg);
            return;
        }
    }
    //输入重定向
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], "<") == 0 )
        {
            input_redirect(account, arg);
            return;
        }
    }
    //追加重定向
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], ">>") == 0 )
        {
            append_redirect(account, arg);
            return;
        }
    }
    */
    //识别系统命令
    sys_command(account, arg);
}

//重定向
void out_in_append(int account, char (*arg)[256])
{
    int i, j = 0;
    char *argv[50];
    char *input = NULL, *output = NULL, *append = NULL;

    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }
    for (i = 0, j = 0; i < account; ++i)
    {
        argv[j] = arg[i];
        if ( strcmp(arg[i], "<") == 0 )
        {
            input = arg[i+1];
            argv[j] = NULL;
            j--;
            ++i;
        }
        if ( strcmp(arg[i], ">") == 0 )
        {
            output = arg[i+1];
            argv[j] = NULL;
            j--;
            ++i;
        }
        if ( strcmp(arg[i], ">>") == 0 )
        {
            append = arg[i+1];
            argv[j] = NULL;
            j--;
            i++;
        }
        j++;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork fails");
    }
    else if (pid > 0)
    {
        if (background == 1)
        {
            return;
        }
        else
        {
            wait(NULL);
        }
    }
    else
    {
        int fd_in, fd_out, fd_append;
        if (input != NULL)
        {
            if ( fd_in = open(input, O_RDONLY) == -1)
                sys_error("open fails");
            dup2(fd_in, STDIN_FILENO);
        }
        if (output != NULL)
        {
            if ( ( fd_out = open(output, O_WRONLY|O_CREAT|O_TRUNC, 0644) ) == -1)
                sys_error("open fails");
            dup2(fd_out, STDOUT_FILENO);
        }
        if (append != NULL)
        {
            if ( ( fd_out = open(append, O_WRONLY) ) == -1 )
                sys_error("open fails");
            dup2(fd_out, STDOUT_FILENO);
        }
        execvp(argv[0], argv);
    }

}

void output_redirect(int account, char (*arg)[256])
{
    int i = 0;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }

    for (i = 0; i < account; ++i)
    {
        argv[i] = arg[i];
        if ( strcmp(arg[i], ">") == 0 )
        {
            argv[i] = NULL;
        }
        if ( strcmp(arg[i], "&") == 0 )
        {
            argv[i] = NULL;
        }
    }
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork fails");
    }
    else if (pid == 0)
    {
        i = 0;
        while (argv[i] != NULL)
        {
            i++;
        }
        i++;
        int fd = open(arg[i], O_RDWR|O_CREAT|O_TRUNC, 0644);
        if (fd == -1)
        {
            sys_error("open fails");
        }
        dup2(fd, STDOUT_FILENO);
        if ( execvp(argv[0], argv) == -1)
        {
            sys_error("execvp fails");
        }
    }
    else
    {   
        if (background == 1)
            return;
        else
            wait(NULL);
    }
}

void input_redirect(int account, char (*arg)[256])
{
    int i;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }

    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], "<") == 0 )
        {
            argv[i] = NULL;
        }
        else
        {
            argv[i] = arg[i];
        }
        if ( strcmp(arg[i], "&") == 0 )
        {
            argv[i] = NULL;
        }
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork fails");
    }
    else if (pid == 0)
    {
        i = 0;
        while (argv[i] != NULL)
        {
            i++;
        }
        i++;
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1)
        {
            sys_error("open fails");
        }
        dup2(fd, STDIN_FILENO);
        if ( execvp(argv[0], argv) == -1)
        {
            sys_error("execvp fails");
        }
    }
    else
    {
        if (background == 1)
        {
            return;
        }
        else
        {
            wait(NULL);
        }
    }
}

void append_redirect(int account, char (*arg)[256])
{
    int i;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }

    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], ">>") == 0 )
        {
            argv[i] = NULL;
        }
        else
        {
            argv[i] = arg[i];
        }
        if ( strcmp(arg[i], "&") == 0 )
        {
            argv[i] = NULL;
        }
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork fails");
    }
    else if (pid == 0)
    {
        i = 0;
        while (argv[i] != NULL)
        {
            i++;
        }
        i++;
        int fd = open(argv[i], O_WRONLY |O_APPEND);
        if (fd == -1)
        {
            sys_error("open fails");
        }
        dup2(fd, STDOUT_FILENO);
        if ( execvp(argv[0], argv) == -1)
        {
            sys_error("execvp fails");
        }
    }
    else
    {
        if (background == 1)
            return;
        else
            wait(NULL);
    }
}

//多重管道2
void command_pipe3(int account, char (*arg)[256])
{
    int pipe_number = 0;
    int i, j;
    char *input = NULL, *output = NULL, *append = NULL;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }

    j = 0;
    for (i = 0; i < account; ++i)
    {
        argv[j] = arg[i];
        if ( strcmp(arg[i], "|") == 0 )
        {
            argv[j] = NULL;
            pipe_number++;
        }
        if ( strcmp(arg[i], "&") == 0 )
        {
            argv[j] = NULL;
        }
        if ( strcmp(arg[i], "<") == 0 )
        {
            input = arg[i+1];
            j--;
            i++;
        }
        if ( strcmp(arg[i], ">") == 0 )
        {
            output = arg[i+1];
            argv[j] = NULL;
            break;
        }
        if ( strcmp(arg[i], ">>") == 0 )
        {
            append = arg[i+1];
            argv[j] = NULL;
            break;
        }
        j++;
    }

    pid_t pid;
    int pipe_fd[50][2];

    //循环创建"|"数个匿名管道
    for (i = 0; i < pipe_number; ++i)
    {
        if ( pipe(pipe_fd[i]) == -1 )
            perror("pipe fails");
    }

    //循环创建"|"数加1个子进程
    for (i = 0; i < pipe_number+1; ++i)
    {
        if ( (pid = fork()) == 0 )
            break;
        if (pid == -1)
        {
            perror("fork fails");
            return;
        }
    }

    if (i == pipe_number+1)
    {
        for (j = 0; j < pipe_number; ++j)
        {
            close(pipe_fd[j][0]);
            close(pipe_fd[j][1]);
        }
        if (background == 1)
        {
            return;
        }
        else
        {
            for (j = 0; j < pipe_number+1; ++j)
            {
                wait(NULL);
            }
        }
    }
    else
    {
        if (i == 0)
        {
            int fd = -1;
            if (input != NULL)
            {
                if ( (fd = open(input, O_RDONLY) ) == -1 )
                {
                    sys_error("open fails");
                }
                dup2(fd, STDIN_FILENO);
            }
            close(pipe_fd[0][0]);
            dup2(pipe_fd[0][1], STDOUT_FILENO);
            for (j = 1; j < pipe_number; ++j)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }
            execvp(argv[0], argv);
            close(pipe_fd[0][1]);
            sys_error("execvp fails");
        }
        else if (i == pipe_number)
        {
            int fd = -1;
            if (output != NULL)
            {
                if ( (fd = open(output, O_WRONLY|O_CREAT|O_TRUNC, 0644) ) == -1 )
                {
                    sys_error("open fails");
                }
                dup2(fd, STDOUT_FILENO);
            }
            else if (append != NULL)
            {
                if ( (fd = open(append, O_WRONLY|O_APPEND) ) == -1 )
                {
                    sys_error("open fails");
                }
                dup2(fd, STDOUT_FILENO);
            }
            close(pipe_fd[pipe_number-1][1]);
            dup2(pipe_fd[pipe_number-1][0], STDIN_FILENO);
            for (j = 0; j < pipe_number-1; ++j)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }
            for (j = 0; j < account; ++j)
            {
                if (argv[j] == NULL)
                {
                    argv[j] = "1";
                    break;
                }
            }
            j++;
            execvp(argv[j], argv+j);
            close(pipe_fd[pipe_number-1][0]);
            sys_error("execvp fails");
        }
        else
        {
            
            close(pipe_fd[i-1][1]);
            close(pipe_fd[i][0]);
            dup2(pipe_fd[i-1][0], STDIN_FILENO);
            dup2(pipe_fd[i][1], STDOUT_FILENO);
            for (j = 0; j < pipe_number; ++j)
            {
                if (j == i-1 | j == i)
                {
                    continue;
                }
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }
            for (j = 0; j < account; ++j)
            {
                if (argv[j] == NULL)
                {
                    argv[j] = "1";
                    break;
                }
            }
            j++;
            execvp(argv[j], argv+j);
            close(pipe_fd[i-1][0]);
            close(pipe_fd[i][1]);
            sys_error("execvp fails");
        }

    }
}

//多重管道1

void command_pipe2(int account, char (*arg)[256])
{
    int pipe_number = 0;
    int i;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }

    for (i = 0; i < account; ++i)
    {
        argv[i] = arg[i];
        if ( strcmp(arg[i], "|") == 0 )
        {
            argv[i] = NULL;
            pipe_number++;
        }
        if ( strcmp(arg[i], "&") == 0 )
        {
            argv[i] = NULL;
        }
    }

    pid_t pid;
    int pipe_fd1[2];
    int pipe_fd2[2];
    int pipe_fd3[2];
    int pipe_fd4[2];
    int pipe_fd5[2];
    if ( pipe(pipe_fd1) == -1 )
    {
        sys_error("pipe fails");
    }
    if ( pipe(pipe_fd2) == -1 )
    {
        sys_error("pipe fails");
    }
    if ( pipe(pipe_fd3) == -1 )
    {
        sys_error("pipe fails");
    }
    if ( pipe(pipe_fd4) == -1 )
    {
        sys_error("pipe fails");
    }
    if ( pipe(pipe_fd5) == -1 )
    {
        sys_error("pipe fails");
    }
    for (i = 0; i < pipe_number+1; ++i)
    {
        if ( (pid = fork()) == 0) 
            break;
        if (pid < 0)
        {
            perror("fork fails");
        }
    }

    if (i == pipe_number+1)
    {
        int j = 0;
        close(pipe_fd1[0]);
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        close(pipe_fd3[0]);
        close(pipe_fd3[1]);
        close(pipe_fd4[0]);
        close(pipe_fd4[1]);
        close(pipe_fd5[0]);
        close(pipe_fd5[1]);
        if (background == 1)
            return;
        else
        {
            while(1)
            {
                if ( waitpid(-1, NULL, WNOHANG) > 0)
                    j++;
                if (j == pipe_number+1)
                    break;
            }
        }
    }
    else if (i == 0 && i < pipe_number)  //子进程0向管道中1写数据
    {
        close(pipe_fd1[0]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        close(pipe_fd3[0]);
        close(pipe_fd3[1]);
        close(pipe_fd4[0]);
        close(pipe_fd4[1]);
        close(pipe_fd5[0]);
        close(pipe_fd5[1]);
        dup2(pipe_fd1[1], STDOUT_FILENO);
        if ( execvp(argv[0], argv) == -1)
        {
            close(pipe_fd1[1]);
            sys_error("execvp fails");
        }
    }
    else if (i == 1 && i < pipe_number)  //子进程1读子进程0写入管道的，并将子进程1的输出写到管道2
    {
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);
        close(pipe_fd3[0]);
        close(pipe_fd3[1]);
        close(pipe_fd4[0]);
        close(pipe_fd4[1]);
        close(pipe_fd5[0]);
        close(pipe_fd5[1]);
        dup2(pipe_fd1[0], STDIN_FILENO);
        dup2(pipe_fd2[1], STDOUT_FILENO);
        for (i = 0; i < account; ++i)
        {
            if (argv[i] == NULL)
            {
                argv[i] = "1";
                break;
            }
        }
        i++;
        if ( execvp(argv[i],  argv + i) == -1 )
        {
            close(pipe_fd1[0]);
            close(pipe_fd2[1]);
            sys_error("execvp fails");
        }
    }
    else if(i == 2 && i < pipe_number) //子进程2读子进程1写入管道的，并将子进程2的输出写入管道3中
    {
        close(pipe_fd1[0]);
        close(pipe_fd1[1]);
        close(pipe_fd2[1]);
        close(pipe_fd3[0]);
        close(pipe_fd4[0]);
        close(pipe_fd4[1]);
        close(pipe_fd5[0]);
        close(pipe_fd5[1]);
        dup2(pipe_fd2[0], STDIN_FILENO);
        dup2(pipe_fd3[1], STDOUT_FILENO);
        for (i = 0; i < account; ++i)
        {
            if (argv[i] == NULL)
            {
                argv[i] = "1";
                break;
            }
        }
        i++;
        if ( execvp(argv[i],  argv + i) == -1 )
        {
            close(pipe_fd2[0]);
            close(pipe_fd3[1]);
            sys_error("execvp fails");
        }
    }
    else if(i == 3 && i < pipe_number) //子进程3读子进程2写入管道的，并将子进程3的输出写入管道4中
    {
        close(pipe_fd1[0]);
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        close(pipe_fd3[1]);
        close(pipe_fd4[0]);
        close(pipe_fd5[0]);
        close(pipe_fd5[1]);
        dup2(pipe_fd3[0], STDIN_FILENO);
        dup2(pipe_fd4[1], STDOUT_FILENO);
        for (i = 0; i < account; ++i)
        {
            if (argv[i] == NULL)
            {
                argv[i] = "1";
                break;
            }
        }
        i++;
        if ( execvp(argv[i],  argv + i) == -1 )
        {
            close(pipe_fd3[0]);
            close(pipe_fd4[1]);
            sys_error("execvp fails");
        }
    }
    else if(i == 4 && i < pipe_number) //子进程4读子进程3写入管道的，并将子进程4的输出写入管道5中
    {
        close(pipe_fd1[0]);
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        close(pipe_fd3[0]);
        close(pipe_fd3[1]);
        close(pipe_fd4[1]);
        close(pipe_fd5[0]);
        dup2(pipe_fd4[0], STDIN_FILENO);
        dup2(pipe_fd5[1], STDOUT_FILENO);
        for (i = 0; i < account; ++i)
        {
            if (argv[i] == NULL)
            {
                argv[i] = "1";
                break;
            }
        }
        i++;
        if ( execvp(argv[i],  argv + i) == -1 )
        {
            close(pipe_fd4[0]);
            close(pipe_fd5[1]);
            sys_error("execvp fails");
        }
    }
    else
    {
        if (i == 1 && i == pipe_number)
        {
            close(pipe_fd1[1]);
            close(pipe_fd2[0]);
            close(pipe_fd2[1]);
            close(pipe_fd3[0]);
            close(pipe_fd3[1]);
            close(pipe_fd4[0]);
            close(pipe_fd4[1]);
            close(pipe_fd5[0]);
            close(pipe_fd5[1]);
            dup2(pipe_fd1[0], STDIN_FILENO);
            for (i = 0; i < account; ++i)
            {
                if (argv[i] == NULL)
                {
                    argv[i] = "1";
                    break;
                }
            }
            i++;
            if ( execvp(argv[i],  argv + i) == -1 )
            {
                close(pipe_fd1[0]);
                sys_error("execvp fails");
            }
        }
        else if (i == 2 && i == pipe_number)
        {
            close(pipe_fd1[0]);
            close(pipe_fd1[1]);
            close(pipe_fd2[1]);
            close(pipe_fd3[0]);
            close(pipe_fd3[1]);
            close(pipe_fd4[0]);
            close(pipe_fd4[1]);
            close(pipe_fd5[0]);
            close(pipe_fd5[1]);
            dup2(pipe_fd2[0], STDIN_FILENO);
            for (i = 0; i < account; ++i)
            {
                if (argv[i] == NULL)
                {
                    argv[i] = "1";
                    break;
                }
            }
            i++;
            if ( execvp(argv[i],  argv + i) == -1 )
            {
                close(pipe_fd2[0]);
                sys_error("execvp fails");
            }
        }
        else if (i == 3 && i == pipe_number)
        {
            close(pipe_fd1[0]);
            close(pipe_fd1[1]);
            close(pipe_fd2[0]);
            close(pipe_fd2[1]);
            close(pipe_fd3[1]);
            close(pipe_fd4[0]);
            close(pipe_fd4[1]);
            close(pipe_fd5[0]);
            close(pipe_fd5[1]);
            dup2(pipe_fd3[0], STDIN_FILENO);
            for (i = 0; i < account; ++i)
            {
                if (argv[i] == NULL)
                {
                    argv[i] = "1";
                    break;
                }
            }
            i++;
            if ( execvp(argv[i],  argv + i) == -1 )
            {
                close(pipe_fd3[0]);
            }
        }
        else if (i == 4 && i == pipe_number)
        {
            close(pipe_fd1[0]);
            close(pipe_fd1[1]);
            close(pipe_fd2[0]);
            close(pipe_fd2[1]);
            close(pipe_fd3[0]);
            close(pipe_fd3[1]);
            close(pipe_fd4[1]);
            close(pipe_fd5[0]);
            close(pipe_fd5[1]);
            dup2(pipe_fd4[0], STDIN_FILENO);
            for (i = 0; i < account; ++i)
            {
                if (argv[i] == NULL)
                {
                    argv[i] = "1";
                    break;
                }
            }
            i++;
            if ( execvp(argv[i],  argv + i) == -1 )
            {
                close(pipe_fd4[0]);
                sys_error("execvp fails");
            }
        }
        else if (i == 4 && i == pipe_number)
        {
            close(pipe_fd1[0]);
            close(pipe_fd1[1]);
            close(pipe_fd2[0]);
            close(pipe_fd2[1]);
            close(pipe_fd3[0]);
            close(pipe_fd3[1]);
            close(pipe_fd4[0]);
            close(pipe_fd4[1]);
            close(pipe_fd5[0]);
            dup2(pipe_fd5[0], STDIN_FILENO);
            for (i = 0; i < account; ++i)
            {
                if (argv[i] == NULL)
                {
                    argv[i] = "1";
                    break;
                }
            }
            i++;
            if ( execvp(argv[i],  argv + i) == -1 )
            {
                close(pipe_fd5[0]);
                sys_error("execvp fails");
            }
        }
    }
}

//单重管道1
void command_pipe1(int account, char (*arg)[256])
{
    int i;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }

    for (i = 0; i < account; ++i)
    {
        argv[i] = arg[i];
        if ( strcmp(arg[i], "|") == 0 )
        {
            argv[i] = NULL;
        }
        if ( strcmp(arg[i], "&") == 0 )
        {
            argv[i] = NULL;
        }
    }

    pid_t pid;
    int pipe_fd[2];
    if ( pipe(pipe_fd) == -1 )
    {
        perror("pipe fails");
    }
    for (i = 0; i < 2; ++i)
    {
        if ( (pid = fork()) == 0) 
            break;
        if (pid < 0)
        {
            perror("fork fails");
        }
    }

    if (i == 0)  //子进程0写
    {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        execvp(argv[0], argv);
        sys_error("execvp fails");
    }
    else if (i == 1)  //子进程1读
    {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        for (i = 0; i < account; ++i)
        {
            if (argv[i] == NULL)
            {
                break;
            }
        }
        i++;
        execvp(argv[i],  argv + i);
        sys_error("execvp fails");
    }
    else
    {
        int j = 0;
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        if (background == 1)
            return;
        else
        {
            while(1)
            {
                if ( waitpid(-1, NULL, WNOHANG) > 0)
                    j++;
                if (j == 2)
                    break;
            }
        }
    }
}

void command_cd(int account, char (*arg)[256])
{
    static char old_cd[256];  //保存上一个工作路径
    static char oold_cd[256]; 
    getcwd(oold_cd, 256);
    if ( strcmp(arg[1], "-") != 0 )
    {
        getcwd(old_cd, 256);
    }
    
    if ( strcmp(arg[1], "~") == 0 )
    {
        if ( chdir("/home/cccmmf") == -1)
            perror("cd fails");
    }
    else if ( strcmp(arg[1], "-") == 0 )
    {
        if ( chdir (old_cd) == -1 )
            perror("cd fails");
        strcpy(old_cd, oold_cd);
    }
    else
    {
        if( chdir(arg[1]) == -1 )
            perror("cd fails");
    }
    
}

void sys_command(int account, char (*arg)[256])
{
    int i;
    char *argv[50];
    for (i = 0; i < 50; ++i)
    {
        argv[i] = NULL;
    }
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], "~") == 0)
        {
            argv[i] = "/home/cccmmf";
        }
        else
        {
            argv[i] = arg[i];
        }
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork fails");
    }
    else if(pid == 0)
    {
        if ( execvp(argv[0],  argv) == -1 )
        {
            sys_error("execvp fails");
        }
    }
    else
    {
        if (background == 1)
            return;
        else
            wait(NULL);
    }
}

void sys_error(char * str)
{
    perror(str);
    exit(-1);
}

void shield_signal(sigset_t sig)
{
    sigfillset(&sig);
    sigprocmask(SIG_BLOCK,&sig,NULL);
}