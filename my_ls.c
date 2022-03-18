// -a、-l、-R、-t、-r、-i、-s 
# include "my_ls.h"

# define LenOfName 256
# define maxN 1005
# define maxM 505
# define maxL 105
# define LenOfPath 256<<4
# define MAX_PATH 1024 
# define MAXNAME 256
int flag = 0;
int num_directory = 0;
int terminalwidth = 0;
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


struct outputFile{
    char FileName[ LenOfName ];
    int modify_time ; 
    int file_type ;
}Output[ maxN ],OutputPoint[ maxM ],Temp[ maxN+maxM ];

int main(int argc,char*argv[])
{
    int i;

	if(argc == 1)
		show(".");
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
    terminalwidth = 80;  
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &wbuf) == -1 || wbuf.ws_col == 0){
        if( tp = getenv("COLUMNS") )
            terminalwidth = atoi( tp );
    }
    else
        terminalwidth = wbuf.ws_col;
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
            show(argv[i]);
        }
    }
    else
    {
        show(".");
    }
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

void show(char *dirname)
{
    char *fullpath;
    DIR *dp;
    struct dirent *dirp;
    struct stat statbuf;
    char filename[200][200];
    int cnt = 0, i = 0, j = 0;

    if ( (dp = opendir(dirname)) == NULL )
    {
        perror("open fails");
        return;
    }

    fullpath = (char *)malloc(sizeof(dirname) + 1 + MAXNAME);
    printf("%s:\n", dirname);
    while ( (dirp = readdir(dp)) != NULL )
    {
        str_num = strlen(dirp->d_name)>str_num ? strlen(dirp->d_name) : str_num;
    }
    getwidth();
    rewinddir(dp);
    if (r_flag || t_flag)
    {
        i = 0;
        while ( (dirp = readdir(dp)) != NULL )
        {
            sprintf(filename[cnt++], "%s", dirp->d_name);
        }
        if (t_flag)
        {

        }
        for (j = cnt-1; j >= 0; --j)
        {
            one_width = 0;
            one_width += str_num;
            sprintf(fullpath, "%s/%s", dirname, filename[j]);
            if ( lstat(fullpath, &statbuf) == -1 )
            {
                perror("Failed to get stat");
                return;
            }
            //控制宽度
            if (i_flag)
            {
                one_width += 8;
            }
            if (s_flag)
            {
                one_width += 5;
            }
            if (i_flag)
            {
                if (!a_flag && filename[j][0] == '.')
                    continue;
                i_information(statbuf.st_ino);
            }
            if (s_flag)
            {
                if (!a_flag && filename[j][0] == '.')
                    continue;
                disk_size(statbuf.st_blocks);
            }
            if (l_flag)
            {
                if (!a_flag && filename[j][0] == '.')
                    continue;
                power(statbuf.st_mode);
                linkk(statbuf.st_nlink);
                uidname(statbuf.st_uid);
                gidname(statbuf.st_gid);
                dirsize(statbuf.st_size);
                revise_time(statbuf.st_mtim);
                dir_name(filename[j]);
                printf("\n");
            }
            else 
            { 
                if (!a_flag && dirp->d_name[0] == '.')
                    continue;
                one_width++;
                if (terminalwidth % one_width == 0)
                    row_num = terminalwidth / one_width;
                else 
                    row_num = terminalwidth / one_width - 1;
                
                printf("%-*s ", str_num, filename[j]);
                i++;
                if (i % row_num == 0)
                    putchar('\n');
            }   
        }
        printf("\n");
    }
    else
    {
        i = 0;
        while ( (dirp = readdir(dp)) != NULL )
        {
            one_width = 0;
            one_width += str_num;
            sprintf(fullpath, "%s/%s", dirname, dirp->d_name);
            if ( lstat(fullpath, &statbuf) == -1)
            {
                perror("Failed to get stat");
                return;
            }
            if (i_flag)
            {
                one_width += 8;
            }
            if (s_flag)
            {
                one_width += 5;
            }
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
                dir_name(dirp->d_name);
                printf("\n");
            }
            else 
            {
                if (!a_flag && dirp->d_name[0] == '.')
                    continue;
                one_width++;
                if (terminalwidth % one_width == 0)
                    row_num = terminalwidth / one_width;
                else 
                    row_num = terminalwidth / one_width - 1;

                printf("%-*s ", str_num+1, dirp->d_name);
                i++;
                if (i % row_num == 0 )
                    putchar('\n');
                
            } 
        }
        printf("\n");
    }
    if (R_flag)
    {
        rewinddir(dp);
        while ( (dirp = readdir(dp)) != NULL )
        {
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
                continue;
            sprintf(fullpath,"%s/%s",dirname,dirp->d_name);
            if ( isadir(fullpath) )
            {
                show(fullpath);
                printf("\n");
            }
        }
    }
    closedir(dp);
    free(fullpath);
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

bool isadir(char *dirname)
{
    struct stat statbuf;
    if ( lstat(dirname, &statbuf) != -1 )
    {
        if (S_ISDIR(statbuf.st_mode))
            return true;
    }
    return false;
}