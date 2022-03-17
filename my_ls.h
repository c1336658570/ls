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
#include<sys/ioctl.h>

void getwidth();
void permission(int len, char *argv[]);
void ls(char *filename);
void ls_a(char *filename);
void ls_t(char *dirname);
void power(int mode);
void linkk(nlink_t link_num);
char* uidname(uid_t uid);
char* gidname(gid_t gid);
void dirsize(off_t ssizee);
void revise_time(struct timespec st_mtimeee);
void dir_name(char *filename);
void i_information(long d_ino);
void disk_size(blkcnt_t blocks);
void show(int len, char *argv[]);
int cmp1( const void *p ,const void *q );
int cmp2( const void *p,const void *q );
void print(char *dir);
void getstat(char *dir);