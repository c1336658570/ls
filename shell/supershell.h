# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <pwd.h>
# include <grp.h>
# include <string.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <error.h>

# define REGULAR 0  //普通命令
# define INPUT_REDIRECT 1 //输入重定向
# define OUTPUT_REDIRECT 2  //输出重定向
# define APPEND_REDIRECT 3 //追加重定向
# define HAVE_PIPE 4 //管道

void ps1(); //输出终端前缀信息
void command_parsing(char *buf, char (*arg)[256], int *commandsize); //命令解析
void do_cmd(int account, char (*arg)[256]);
void sys_error(char * str);  //错误处理