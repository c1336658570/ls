/*
实现 管道 (也就是 |)
实现 输入输出重定向(也就是 < > >>)
实现 后台运行（也就是 & ）
实现 cd，要求支持能切换到绝对路径，相对路径和支持 cd -
屏蔽一些信号（如 ctrl + c 不能终止）
*/

int background;

# include "supershell.h"

int main(void)
{
    int account, i;
    char *buf;
    char argvlist[50][256];

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

    printf("\033[35m%s\033[0m:\033[31m%s\033[0m$ ", name, path);

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
            background = 1;
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
    //重定向输出
    for (i = 0; i < account; ++i)
    {
        if ( strcmp(arg[i], ">") == 0 )
        {
            output_redirect(account, arg);
            return;
        }
    }
    //重定向输入
}

void output_redirect(int account, char (*arg)[256])
{
    int i = 0;
    char *argv[50];
    for (i = 0; i < account; ++i)
    {
        argv[i] = arg[i];
        if ( strcmp(arg[i], ">") == 0 )
        {
            argv[i] = NULL;
        }
    }
    pid_t pid = fork();

    if (pid < 0)
    {
        sys_error("fork fails");
    }
    else if (pid == 0)
    {
        if (background == 1)
            return;
        else
            wait(NULL);
    }
    else
    {
        i = 0;
        while (argv[i] != NULL)
        {
            i++;
        }
        i++;
        int fd = open(arg[i], O_RDWR|O_CREAT|O_TRUNC, 777);
        if (fd == -1)
        {
            sys_error("open fails");
        }
        sleep(2);
        dup2(fd, STDOUT_FILENO);
        execvp(argv[0], argv);
    }
}

void input_redirect(int account, char (*arg)[256])
{

}

void append_redirect(int account, char (*arg)[256])
{

}

void command_pipe(int account, char (*arg)[256])
{

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
        chdir("/home/cccmmf");
    }
    else if ( strcmp(arg[1], "-") == 0 )
    {
        chdir (old_cd);
        strcpy(old_cd, oold_cd);
    }
    else
    {
        chdir(arg[1]);
    }
    
}

void Background(int account, char (*arg)[256])
{

}

void sys_error(char * str)
{
    perror(str);
    exit(-1);
}