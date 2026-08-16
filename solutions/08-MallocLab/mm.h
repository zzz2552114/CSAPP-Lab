#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);
/* 中文：四个必须由学生在 mm.c 中实现的分配器接口函数 */


/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
/* 中文翻译：
 * 学生以一人或两人小组形式工作。小组在 mm.c 文件的这种
 * 结构体里填写他们的团队名、个人姓名和登录 ID。
 */
typedef struct {
    char *teamname; /* ID1+ID2 or ID1 */
    char *name1;    /* full name of first member */
    char *id1;      /* login ID of first member */
    char *name2;    /* full name of second member (if any) */
    char *id2;      /* login ID of second member */
} team_t;
/* 中文：teamname = 团队名（ID1+ID2 或 ID1）；name1 = 第一个成员全名；
   id1 = 第一个成员登录 ID；name2 = 第二个成员全名（如果有）；id2 = 第二个成员登录 ID */

extern team_t team;

