/*
 * CS:APP Data Lab
 */

/* Declare different function types */
/* 中文：声明不同的函数指针类型（分别对应 0、1、2、3 个参数的函数） */
typedef int (*funct_t) (void);
typedef int (*funct1_t)(int);
typedef int (*funct2_t)(int, int); 
typedef int (*funct3_t)(int, int, int); 

/* Combine all the information about a function and its tests as structure */
typedef struct {
    char *name;             /* String name */
    funct_t solution_funct; /* Function */
    funct_t test_funct;     /* Test function */
    int args;               /* Number of function arguments */
    char *ops;              /* List of legal operators. Special case: "$" for floating point */
    int op_limit;           /* Max number of ops allowed in solution */
    int rating;             /* Problem rating (1 -- 4) */
    int arg_ranges[3][2];   /* Argument ranges. Always defined for 3 args, even if */
                            /* the function takes fewer. Special case: First arg */
			    /* must be set to {1,1} for f.p. puzzles */
} test_rec, *test_ptr;

/*
 * 中文说明：test_rec —— 汇总"一个谜题函数及其测试"的全部信息。
 *   name           函数名（字符串）
 *   solution_funct 学生实现的函数指针
 *   test_funct     参考测试函数指针（表达正确行为的标准答案）
 *   args           函数参数的个数
 *   ops            允许的操作符列表；特殊值 "$" 表示浮点谜题
 *   op_limit       允许的最大操作符数
 *   rating         题目分值（1~4）
 *   arg_ranges[3][2] 每个参数的取值范围。总是按 3 个参数定义（即使函数参数更少）。
 *                    特殊约定：浮点谜题的第一个参数必须设为 {1,1}
 */

extern test_rec test_set[];







