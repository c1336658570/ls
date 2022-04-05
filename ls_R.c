# include <sys/types.h>
# include <sys/stat.h>
# include <stdio.h>
# include <dirent.h>
# include <unistd.h>
# include <string.h>

#define MAX_PATH 1024 

void print(char *dir);
void getstat(char *dir);

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        getstat(".");
    }
    else
    {
        while (--argc)
        {
            getstat(*++argv);
        }
    }
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