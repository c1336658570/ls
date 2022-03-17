// -a、-l、-R、-t、-r、-i、-s 
# include "my_ls.h"

#define LenOfName 256
#define maxN 1005
#define maxM 505
#define maxL 105
#define LenOfPath 256<<4
#define MAX_PATH 1024 
int flag = 0;
int num_directory = 0;
int terminalWidth;
bool have_directory = false;
bool a_flag = false;
bool l_flag = false;
bool R_flag = false;
bool t_flag = false;
bool r_flag = false;
bool i_flag = false;
bool s_flag = false;


struct outputFile{
    char FileName[ LenOfName ];
    int modify_time ; 
    int file_type ;
}Output[ maxN ],OutputPoint[ maxM ],Temp[ maxN+maxM ];

int main(int argc,char*argv[])
{
    int i;

	if(argc == 1)
		ls(".");
	else
	{
        for (i = 1; i < argc; ++i)
        {
            if (argv[i][0] == '-')
            {
                flag++;
            }
            else
            {
                break;
            }
        }
        if (argc - flag - 1 > 0)
        {
            num_directory = argc - flag - 1;
            have_directory = true;
        }
        permission(argc, argv);
    }
	return 0;
}

//获取终端长度
void getwidth()
{
    char *tp;
    struct winsize wbuf;
    terminalWidth = 80;  
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &wbuf) == -1 || wbuf.ws_col == 0){
        if( tp = getenv("COLUMNS") )
            terminalWidth = atoi( tp );
    }
    else
        terminalWidth = wbuf.ws_col;
    return;
}

//参数解析
void permission(int len, char *argv[])
{
    int i, j;
    int length = len;
    if (have_directory == true)
    {
        length = length - num_directory; 
    }
    for (i = 1; i < length; i++)
    {
        for (j = 0; j < strlen(argv[i]); j++)
        {
            if (argv[i][0] != '-')
            {
                printf("Parameter erro,%s\n", argv[i]);
                exit(-1);
            }
            // -a、-l、-R、-t、-r、-i、-s 
            if (argv[i][j] == 'i')
            {
                i_flag = true;
            }
            else if (argv[i][j] == 'l')
            {
                l_flag = true;
            }
            else if (argv[i][j] == 'a')
            {
                a_flag = true;
            }
            else if (argv[i][j] == 'R')
            {
                R_flag = true;
            }
            else if (argv[i][j] == 't')
            {
                t_flag = true;
            }
            else if (argv[i][j] == 'r')
            {
                r_flag = true;
            }
            else if (argv[i][j] == 's')
            {
                s_flag = true;
            }
            else if (j != 0)
            {
                printf("Illegal option,%s\n", argv[i]);
                exit(-1);
            }
        }
    }
    if (have_directory == true)
    {
        for (int i = (len - 1); i >= (len - num_directory); i--)
        {
            show(len, argv);
        }
    }
    else
    {
        show(len, argv);
    }
}

//无任何参数
void ls(char *filename)
{
    DIR *dp;
    struct dirent *dirp;
    
    if ( (dp = opendir(filename)) == NULL )
        perror("opendir fail");
    while ( (dirp = readdir(dp)) != NULL )
    {
        if (dirp->d_name[0] == '.')
            continue;
        printf("%s\n", dirp->d_name);
    }
    closedir(dp);

}

//ls_a
void ls_a(char *filename)
{
    DIR *dp;
    struct dirent *dirp;

    if ( (dp = opendir(filename)) == NULL )
        perror("opendir fail");
    while ( ( dirp = readdir(dp) ) != NULL )
        printf("%s\n", dirp->d_name);
    
    closedir(dp);
}

void ls_t(char *dirname)
{
    int cnt = 0;
    DIR *dp;
    struct dirent *dirp;
    if ( (dp = opendir(dirname)) == NULL )
        perror("opendir fails"); 
    while ( (dirp = readdir(dp)) != NULL )
    {
        cnt++;
    }
    char arr[cnt][256];
}

//输出文件类型和权限
void power(int mode)
{
    char str[11];
	strcpy(str,"----------");
	if (S_ISDIR(mode)) str[0] = 'd';
    if (S_ISLNK(mode)) str[0] = 'l';
    if (S_ISCHR(mode)) str[0] = 'c';
    if (S_ISBLK(mode)) str[0] = 'b';
    if (S_ISFIFO(mode)) str[0] = 'p';
    if (S_ISSOCK(mode)) str[0] = 's';
	
	if (mode & S_IRUSR) str[1] = 'r';
	if (mode & S_IWUSR) str[2] = 'w';
	if (mode & S_IXUSR) str[3] = 'x';
	
	if (mode & S_IRGRP) str[4] = 'r';
	if (mode & S_IWGRP) str[5] = 'w';
	if (mode & S_IXGRP) str[6] = 'x';
	
	if (mode & S_IROTH) str[7] = 'r';
	if (mode & S_IWOTH) str[8] = 'w';
	if (mode & S_IXOTH) str[9] = 'x';
    printf("%s", str);
}

//输出链接数
void linkk(nlink_t link_num)
{
    printf("%4d ", (int)link_num);
}

//输出用户名
char* uidname(uid_t uid)
{
	struct passwd *user;
	if ( (user = getpwuid(uid)) == NULL )
	{
		printf("%-8d ", uid);
	}
	else
    {
        printf("%-8s ", user->pw_name);
    }
}

//输出组名
char* gidname(gid_t gid)
{
	struct group * grp;
	
	if ( (grp = getgrgid(gid)) == NULL )
	{
		printf("%-8d ", gid);
    }
	else
	{
        printf("%-8s ", grp->gr_name);
    }
}

//输出文件大小
void dirsize(off_t ssizee)
{
    printf("%8ld ", (long)ssizee);
}

//输出最后修改时间
void revise_time(struct timespec st_mtimeee)
{
    printf("%.12s ", ctime(&st_mtimeee) + 4);
}

//输出文件名
void dir_name(char *filename)
{
    printf("%s", filename);
}

//输出i节点号
void i_information(long d_ino)
{
    printf("%-7ld ", d_ino);
}

//输出磁盘块大小
void disk_size(blkcnt_t blocks)
{
    printf("%-4ld ", blocks);
}

void show(int len, char *argv[])
{
    char name[MAX_PATH];
    static int i;
    i = flag;
    DIR *dp;
    struct dirent *dirp;
    struct stat statbuf;
    if (num_directory == 0)
    {
        if ( (dp = opendir(".") ) == NULL )
        {
            perror("open fails");
            return;
        }
    }
    else
    {
        if ( (dp = opendir(argv[++i])) == NULL )
        {
            perror("open fails");
            return;
        }
    }
    while ( (dirp = readdir(dp)) != NULL )
    {
        sprintf(name, "%s/%s", argv[i], dirp->d_name);
        if ( lstat(name, &statbuf) == -1)
        {
            perror("Failed to get stat");
            return;
        }

        if (!R_flag && !t_flag && !r_flag)
        {
            if (i_flag)
            {
                if (!a_flag && dirp->d_name[0] == '.')
                    continue;
                i_information(statbuf.st_ino);
            }
            if (s_flag)
            {
                if (!a_flag && dirp->d_name[0] == '.')
                    continue;
                disk_size(statbuf.st_blocks);
            }
            if (l_flag)
            {
                if (!a_flag && dirp->d_name[0] == '.')
                    continue;
                power(statbuf.st_mode);
                linkk(statbuf.st_nlink);
                uidname(statbuf.st_uid);
                gidname(statbuf.st_gid);
                dirsize(statbuf.st_size);
                revise_time(statbuf.st_mtim);
            }
            dir_name(dirp->d_name);
            printf("\n");
        }
    }
}

//文件名排序
int cmp1( const void *p ,const void *q )
{
    char T1[ LenOfName ],T2[ LenOfName ];
    strcpy( T1,(*(struct outputFile *)p).FileName );
    strcpy( T2,(*(struct outputFile *)q).FileName );
    int len1 = strlen( T1 );
    int i ;
    for( i=0;i<len1;i++ ){
        if( T1[ i ]>='A' && T1[ i ]<='Z' ){
            T1[ i ] = T1[ i ] - 'A' + 'a';
        }
    }
    int len2 = strlen( T2 );
    for( i=0;i<len2;i++ ){
        if( T2[ i ]>='A' && T2[ i ]<='Z' ){
            T2[ i ] = T2[ i ] - 'A' + 'a';
        }
    }
    return strcmp( T1,T2 );
}
 
//修改时间排序
int cmp2( const void *p,const void *q )
{
    return (*(struct outputFile *)p).modify_time < (*(struct outputFile *)q).modify_time;
}

void print(char *dir)
{
    char name[MAX_PATH];
    struct dirent *dirp;
    DIR *dp;

    if ( (dp = opendir (dir) ) == NULL )
    {
        fprintf(stderr, "open fail");
        return;
    }
    while ( (dirp = readdir(dp)) != NULL )
    {
        if ( !strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
            continue;
        
        if (strlen(dir) + strlen(dirp->d_name) + 2 > MAX_PATH)
        {
            fprintf(stderr, "name too long");
            return;
        }
        sprintf(name, "%s/%s", dir, dirp->d_name);
        getstat(name);
    }
    closedir(dp);

}

void getstat(char *dir)
{
    struct stat statbuf;

    if ( ( lstat(dir, &statbuf) ) == -1 )
    {
        fprintf(stderr, "failed to get stat(%s)", dir);
        return;
    }
    if ( S_ISDIR(statbuf.st_mode) )
    {
        print(dir);
    }
    printf("%8ld %s\n", statbuf.st_size, dir);

    return;
}