/*************************************************************************
	> File Name: part1.c
	> Author: 
	> Mail: 
	> Created Time: 2022年01月13日 星期四 13时33分10秒
 ************************************************************************/

# include "strbuf.h"
# include <ctype.h>

//初始化sb结构体，容量为alloc。
void strbuf_init(struct strbuf *sb, size_t alloc)
{
    if (alloc == 0)
    {
        sb->buf = (char*)malloc(sizeof(char));
        sb->alloc = 0;
    }
    else
    {
        sb->buf = (char*)malloc(alloc * sizeof(char));   
        sb->alloc = alloc;
    }
    sb->buf[0] = '\0';
    sb->len = 0;
}

//将字符串填充到sb中，长度为len，容量为alloc。
void strbuf_attach(struct strbuf *sb, void *str, size_t len,size_t alloc)
{
    sb->alloc = alloc;
    sb->buf = (char *)str;
    sb->len = len;
}

//释放sb结构体的内存。
void strbuf_release(struct strbuf *sb)
{
    free(sb->buf);
    sb->buf = NULL;
    sb->alloc = 0;
    sb->len = 0;
}


//交换两个 strbuf。
void strbuf_swap(struct strbuf *a, struct strbuf *b)
{
    char *c = a->buf;
    a->buf = b->buf;
    b->buf = c;
    int t = a->len;
    a->len = b->len;
    b->len = t;
    t = a->alloc;
    a->alloc = b->alloc;
    b->alloc = t;
}

//将 sb 中的原始内存取出，并获得其长度。
char *strbuf_detach(struct strbuf *sb, size_t *sz)
{
    (*sz) = sb->alloc;
    return sb->buf;
    
}

//比较两个 strbuf 的内存是否相同。
int strbuf_cmp(const struct strbuf *first, const struct strbuf *second)
{
    if (first->len == second->len)
        return 0;
    else if (first->len < second->len)
        return -1;
    else
        return 1;
}

//清空 sb。
void strbuf_reset(struct strbuf *sb)
{
    sb->len = 0;
}

//将 sb 的长度扩大 extra。
void strbuf_grow(struct strbuf *sb, size_t extra)
{
    if (extra == 0 || sb->len+extra < sb->alloc)
        return;
    sb->buf = (char *)realloc(sb->buf, sb->len+extra);
    sb->alloc = sb->len+extra;
}

//向 sb 追加长度为 len 的数据 data。
void strbuf_add(struct strbuf *sb, const void *data, size_t len)
{
    if (len == 0)
        return;
    while (sb->len+len >= sb->alloc)
    {
        sb->buf = (char *)realloc(sb->buf, sb->alloc*2);
        sb->alloc *= 2; 
    }
    if (len == 1)
    {
        sb->buf[sb->len] = *(char *)data;
        sb->buf[sb->len+1] = '\0';
    }
    else
        strcat(sb->buf+sb->len, (char *)data);
    sb->len = strlen(sb->buf);
}

//向 sb 追加一个字符 c。
void strbuf_addch(struct strbuf *sb, int c)
{
    while (sb->len+1 >= sb->alloc)
    {
        sb->buf = (char *)realloc(sb->buf, sb->alloc << 1);
        sb->alloc <<= 1; 
    }
    sb->buf[sb->len] = c;
    sb->buf[sb->len+1] = '\0';
    sb->len++;
}

//向 sb 追加一个字符串 s。
void strbuf_addstr(struct strbuf *sb, const char *s)
{
    while (sb->len+strlen(s) >= sb->alloc)
    {
        sb->buf = (char *)realloc(sb->buf, (sb->alloc+1) << 1);
        sb->alloc = (sb->alloc+1)*2; 
    }
    strcat(sb->buf,s);
    sb->len += strlen(s);
}

//向一个 sb 追加另一个 strbuf的数据。
void strbuf_addbuf(struct strbuf *sb, const struct strbuf *sb2)
{
    while (sb->len+sb2->len >= sb->alloc)
    {
        sb->buf = (char *)realloc(sb->buf, sb->len+sb2->len+1);
        sb->alloc = sb->len+sb2->len+1;
    }
    strcat(sb->buf+sb->len, sb2->buf);
    sb->len += sb2->len;
}

//设置 sb 的长度 len。
void strbuf_setlen(struct strbuf *sb, size_t len)
{
    if (sb->alloc < sb->len)
    {
        sb->buf = (char *)realloc(sb->buf,len+1);
        sb->alloc = len;
    }
    sb->len = len;
    sb->buf[len] = '\0';
}

//计算 sb 目前仍可以向后追加的字符串长度。
size_t strbuf_avail(const struct strbuf *sb)
{
    return (sb->alloc - sb->len-1);
}

//向 sb 内存坐标为 pos 位置插入长度为 len 的数据 data。
void strbuf_insert(struct strbuf *sb, size_t pos, const void *data, size_t len)
{
    if (len == 0)
        return;
    while (sb->len+len >= sb->alloc)
    {
        sb->buf = (char *)realloc(sb->buf, sb->alloc << 1);
        sb->alloc <<= 1; 
    }
    if (pos > sb->len)
    {
        strcat(sb->buf, (char *)data);
    }
    char arr[sb->len - pos+1];
    strcpy(arr, sb->buf+pos);
    sb->buf[pos] = '\0';
    strcat(sb->buf, (char *)data);
    strcat(sb->buf, arr);
    sb->len = strlen(sb->buf);
}

//去除 sb 缓冲区左端的所有 空格，tab, '\t'。
void strbuf_ltrim(struct strbuf *sb)
{
    char *a = sb->buf;
    int i = 0;
    char arr[sb->len+1];
    while (isspace(a[i]))
    {
        a += 1;
        i++;
    }
    a++;
    strcpy(arr, a);
    strcpy(sb->buf, arr);
    sb->len = strlen(sb->buf);
}

//去除 sb 缓冲区右端的所有 空格，tab, '\t'。
void strbuf_rtrim(struct strbuf *sb)
{
    char *a = sb->buf+sb->len-1;
    int i = 0;
    while (isspace(*a))
    {
        a -= 1;
        i++;
    }
    sb->buf[sb->len-i] = '\0';
    sb->len = strlen(sb->buf);
}

//删除 sb 缓冲区从 pos 坐标长度为 len 的内容。
void strbuf_remove(struct strbuf *sb, size_t pos, size_t len)
{
    if (pos + sb->len > sb->alloc)
        sb->buf[pos] = '\0';
    strcpy(sb->buf+pos, sb->buf+len+2);
    sb->len = strlen(sb->buf);
}

//为 sb 直接扩容 hint ? hint : 8192 大小， 
//然后将文件描述符为 fd 的所有文件内容读取到 sb 中。
ssize_t strbuf_read(struct strbuf *sb, int fd, size_t hint)
{
    FILE *fp = NULL;
    fp = fdopen(fd, "r");
    if (NULL == fp)
    {
        exit(1);
    }
    char ch;
    ssize_t i = 0;
    while ( (ch = fgetc(fp)) != EOF )
    {
        while (sb->alloc <= sb->len+1)
        {
            sb->buf = (char *)realloc(sb->buf, sb->len+(hint ? hint : 8192) );
            sb->alloc += (hint ? hint : 8192);
        }
        strbuf_addch(sb, ch);
        i++;
    }
    return i;
}

// //将 将文件句柄为 fp 的一行内容读取到 sb 。
int strbuf_getline(struct strbuf *sb, FILE *fp)
{
    char ch;
    int i = 0;
    while ( (ch = fgetc(fp)) != EOF )
    {
        if (ch == '\n')
            break;
        strbuf_add(sb, &ch, 1);
        i++;
    }
    return i;
}

// //将长度为 len 的字符串 str 根据切割字符 terminator 切成多个 strbuf,并从结果返回，
// //max 可以用来限定最大切割数量。返回 struct strbuf 的指针数组，数组的最后元素为 NULL
struct strbuf **strbuf_split_buf(const char *str, size_t len, int terminator, int max)
{  
    if (len == 0)
        return NULL;
    
    struct strbuf *t;
    struct strbuf ** sb = (struct strbuf **)malloc(sizeof(struct strbuf *));

    int i = 0, k = 2;
    char arr[2] = {(char)terminator, '\0'};
    char c[len+1];

    memcpy(c, str, len);
    c[len] = '\0';
    for (i = 0; i < len; ++i)
    {
        if (c[i] == '\0')
        {
            c[i] = '\a';
        }
    }


    char *p, *q;
    q = strtok_r(c, arr, &p); 


    while(q != NULL && (k-2) < max)
    {
        t = (struct strbuf *)malloc(sizeof(struct strbuf));
        strbuf_init(t,strlen(q)+1);
        strbuf_addstr(t,q);
        for (i = 0; i < strlen(q); ++i)
        {
            if(t->buf[i] == '\a')
                t->buf[i] ='\0';
        }

        sb = (struct strbuf **)realloc(sb,sizeof(struct strbuf*)*k);
        sb[k-2] = t;
        k++;
        q = strtok_r(NULL,arr, &p);
    }
    sb[k-2] = NULL;
    return sb;

/* 
    if (len == 0)
        return NULL;
    
    struct strbuf ** sb = (struct strbuf **)malloc(sizeof(struct strbuf *));
    char *arr = (char *)malloc(len+5);
    long i = 0, j = 0, k = 2, p = 0;
    for (i = 0; i < len; ++i)
    {
        arr[i] = str[i];
    }
    arr[len] = '\0';
    i = 0;
    p = j = (long)arr;
    while (j < ((long)arr)+len)
    {
        if ( (j+max) > (long)memchr(arr, terminator, len) )
        {
            j = (long)((char *)memchr(arr, terminator, len))+1;
            *((char *)(j-1)) = '\0';
            sb[k-2]->len = j-p;
            sb[k-2]->alloc = j-p;
            sb[k-2]->buf = (char *)p;
            sb = (struct strbuf**)realloc(sb, sizeof(struct strbut *)*k);
            k++;

            p = j;
        }
        else
        {
            j += max;
            sb[k-2]->len = max;
            sb[k-2]->alloc = max;
            sb[k-2]->buf = (char *)malloc(max+1);
            sb[k-2]->buf[max] = '\0';
            memcpy(sb[k-2]->buf, (char *)p, max);
            sb = (struct strbuf**)realloc(sb, sizeof(struct strbut *)*k);
            k++;
            
            p = j;
        }
    }
    sb[k-2] = NULL;
    return sb;
*/
}

// //target_str : 目标字符串，str : 前缀字符串，strlen : target_str 长度 ，
// //前缀相同返回 true 失败返回 false
bool strbuf_begin_judge(char *target_str, const char *str, int strnlen)
{
    if (strnlen == 0)
        return true;
    int i = 0;
    char ch = target_str[strlen(str)];
    target_str[strlen(str)] = '\0';
    if ( !strcmp(target_str, str) )
        i = 1;
    target_str[strlen(str)] = ch;

    return i;
}

// //target_str : 目标字符串，begin : 开始下标，end 结束下标。len : target_buf的长度，
// //参数不合法返回 NULL. 下标从0开始，[begin, end]区间。
char *strbuf_get_mid_buf(char *target_buf, int begin, int end, int len)
{
    if (begin < 0 || end >= len || end < begin)
        return NULL;
    char *a = (char *)malloc(end-begin+1);
    char ch = target_buf[end+1];
    target_buf[end] = '\0';
    strcpy(a, target_buf+begin);

    target_buf[end] = ch;
    return a;
}