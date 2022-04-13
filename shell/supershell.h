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
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
# include <sys/wait.h>

void ps1(); //输出终端前缀信息
void command_parsing(char *buf, char (*arg)[256], int *commandsize); //命令解析
void do_cmd(int account, char (*arg)[256]);  //执行命令
void command_pipe(int account, char (*arg)[256]);  //管道
void input_redirect(int account, char (*arg)[256]);  //输入重定向
void output_redirect(int account, char (*arg)[256]);  //输出重定向
void append_redirect(int account, char (*arg)[256]);  //追加重定向
void sys_command(int account, char (*arg)[256]);  //系统内置命令
void command_cd(int account, char (*arg)[256]);  //cd
void sys_error(char * str);  //错误处理