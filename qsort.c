# include <stdio.h>
# include <stdlib.h>
# include <string.h>

struct Stu
{
	int age;
	char name[20];
};
//比较结构体中元素的年龄
int cmp_age(const void* e1, const void* e2)
{
	//本质是比较整形
	return ((struct Stu*)e1)->age - ((struct Stu*)e2)->age;
}
//比较名字
int cmp_name(const void* e1, const void* e2)
{
	//本质是字符串比较->使用strcmp函数
	return strcmp(((struct Stu*)e1)->name, ((struct Stu*)e2)->name);
}
void test2()
{
	//创建结构体数组，用大括号初始化
	struct Stu s[5] = { {14,"Mango"},{18,"Lemon"},{13,"Hello"}, {21, "fff"}, {5, "asd"} };
	int sz = sizeof(s) / sizeof(s[0]);
	//以年龄排
	qsort(s, sz, sizeof(s[0]), cmp_age);
	printf("%s %d ",s[0].name,s[0].age);
	printf("%s %d ", s[1].name, s[1].age);
	printf("%s %d ", s[2].name, s[2].age);
    printf("%s %d ", s[3].name, s[3].age);
	printf("%s %d ", s[4].name, s[4].age);

	printf("\n");
	//以姓名排
	qsort(s, sz, sizeof(s[0]), cmp_name);
	printf("%s %d ", s[0].name, s[0].age);
	printf("%s %d ", s[1].name, s[1].age);
	printf("%s %d ", s[2].name, s[2].age);
    printf("%s %d ", s[3].name, s[3].age);
	printf("%s %d ", s[4].name, s[4].age);
	printf("\n");
}

int main(void)
{
    test2();
}