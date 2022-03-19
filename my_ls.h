# include <sys/stat.h>
# include <stdio.h>
# include <dirent.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <pwd.h>
# include <grp.h>
# include <time.h>
# include <stdbool.h>
# include<sys/ioctl.h>
#include <fcntl.h>

# define LenOfName 256
# define maxN 1005
# define maxM 505
# define maxL 105
# define MAX_PATH 1024 
# define MAXNAME 256
int flag = 0;
int num_directory = 0;
int terminalwidth = 120;
int str_num = 0;
int row_num = 0;
int one_width = 0;
int total_length = 0;
bool have_directory = false;
bool a_flag = false;
bool l_flag = false;
bool R_flag = false;
bool t_flag = false;
bool r_flag = false;
bool i_flag = false;
bool s_flag = false;

struct file{
    char FileName[MAXNAME];
    int  modify_time; 
};

void getwidth();
void permission(int len, char *argv[]);
void power(int mode);
void linkk(nlink_t link_num);
char* uidname(uid_t uid);
char* gidname(gid_t gid);
void dirsize(off_t ssizee);
void revise_time(struct timespec st_mtimeee);
void dir_name(char *filename, int mode);
void i_information(long d_ino);
void disk_size(blkcnt_t blocks);
void show(char *dirname);
int cmp( const void *p ,const void *q );
bool isadir(char *dirname);
void QuickSort(struct file *a, int low, int high);
int FindPos(struct file *a, int low, int high);