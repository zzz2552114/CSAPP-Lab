/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

/*
 * 中文说明：
 *   bits.c —— 存放你 Data Lab 解答的源文件，也是最终要交给老师的文件。
 *   警告：不要 #include <stdio.h>，否则会干扰 dlc 编译器。不包含 <stdio.h>
 *   时你仍然可以在调试时使用 printf（gcc 会警告，可忽略）。一般来说忽略编译
 *   警告不是好习惯，但在这个实验里没关系。
 *   记得把 <Please put your name and userid here> 换成你的姓名和学号。
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */

/* ============================================================
 * 中文翻译（对应上文 Instructions to Students 的全部内容）
 * ============================================================
 *
 * 你通过编辑本源文件中的一系列函数，提交你对 Data Lab 的解答。
 *
 * 整数谜题编码规则：
 *   - 用一行或多行 C 代码实现每个函数，并替换其中的 "return" 语句。
 *     你的代码必须符合下面的风格：
 *         int Funct(arg1, arg2, ...) {
 *             此处写简短的实现思路说明
 *             int var1 = Expr1;
 *             ...
 *             int varM = ExprM;
 *             varJ = ExprJ;
 *             ...
 *             varN = ExprN;
 *             return ExprR;
 *         }
 *   - 每个 "Expr" 只能使用：
 *       1. 0 到 255（0xFF）之间的整型常量，禁止使用 0xffffffff 这样的大常量。
 *       2. 函数参数和局部变量（不允许使用全局变量）。
 *       3. 一元整型操作符：! ~
 *       4. 二元整型操作符：& ^ | + << >>
 *   - 有些题目会进一步限制允许的操作符。
 *   - 每个 "Expr" 可以包含多个操作符，不限于每行一个。
 *   - 明确禁止：
 *       1. 使用任何控制结构（if、do、while、for、switch 等）。
 *       2. 定义或使用任何宏。
 *       3. 在本文件中定义额外的函数。
 *       4. 调用任何函数。
 *       5. 使用任何其他操作，如 &&、||、-、?:。
 *       6. 使用任何形式的类型转换（casting）。
 *       7. 使用除 int 以外的任何数据类型（因此不能用数组、struct 或 union）。
 *   - 你可以假设你的机器：
 *       1. 使用 32 位二进制补码表示整数。
 *       2. 右移采用算术右移。
 *       3. 当移位量小于 0 或大于 31 时，移位行为不可预测。
 *
 * 可接受编码风格的示例：
 *   pow2plus1 —— 返回 2^x + 1（0 <= x <= 31）
 *   pow2plus4 —— 返回 2^x + 4（0 <= x <= 31）
 *
 * 浮点谜题编码规则：
 *   实现浮点运算的题目规则较宽松：允许使用循环和条件控制，允许同时使用
 *   int 和 unsigned，允许使用任意整型/无符号常量，允许对 int 或 unsigned
 *   数据使用任何算术、逻辑或比较操作。
 *   明确禁止：
 *       1. 定义或使用任何宏。
 *       2. 在本文件中定义额外的函数。
 *       3. 调用任何函数。
 *       4. 使用任何形式的类型转换。
 *       5. 使用除 int 或 unsigned 以外的任何数据类型（不能用数组、struct、union）。
 *       6. 使用任何浮点数据类型、浮点操作或浮点常量。
 *
 * 备注：
 *   1. 用 dlc（data lab checker）检查你的解法是否合法。
 *   2. 每个函数都有允许使用的最大操作数（整型/逻辑/比较操作），dlc 会检查
 *      是否超限。注意：赋值号 '=' 不计入操作数，你可以随意使用。
 *   3. 用 btest 测试框架检查你的函数是否正确。
 *   4. 用 BDD checker 对你的函数进行形式化验证。
 *   5. 每个函数的最大操作数写在每个函数的头注释中；如果 writeup 与本文件
 *      给出的最大操作数不一致，以本文件为准。
 *
 * 步骤 2：按照编码规则修改下面的函数。
 *   重要——为避免评分时出现意外：
 *   1. 用 dlc 检查你的解法是否符合编码规则。
 *   2. 用 BDD checker 形式化验证你的解法能产生正确答案。
 * ============================================================ */

#endif
//1
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
/* 中文：bitXor(x, y) —— 仅用 ~ 和 & 实现 x^y（按位异或）
 *   示例：bitXor(4, 5) = 1
 *   允许操作符：~ &
 *   最多操作符：14
 *   分值：1
 */
int bitXor(int x, int y) {
  return 2;
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
/* 中文：tmin() —— 返回最小的补码整数
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：4
 *   分值：1
 */
int tmin(void) {

  return 2;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
/* 中文：isTmax(x) —— 若 x 是最大的补码整数则返回 1，否则返回 0
 *   允许操作符：! ~ & ^ | +
 *   最多操作符：10
 *   分值：1
 */
int isTmax(int x) {
  return 2;
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
/* 中文：allOddBits(x) —— 若 x 的所有奇数位都为 1，返回 1
 *   位编号从 0（最低有效位）到 31（最高有效位）
 *   示例：allOddBits(0xFFFFFFFD) = 0，allOddBits(0xAAAAAAAA) = 1
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：12
 *   分值：2
 */
int allOddBits(int x) {
  return 2;
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
/* 中文：negate(x) —— 返回 -x
 *   示例：negate(1) = -1
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：5
 *   分值：2
 */
int negate(int x) {
  return 2;
}
//3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
/* 中文：isAsciiDigit(x) —— 若 0x30 <= x <= 0x39（字符 '0'~'9' 的 ASCII 码）返回 1
 *   示例：isAsciiDigit(0x35) = 1
 *         isAsciiDigit(0x3a) = 0
 *         isAsciiDigit(0x05) = 0
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：15
 *   分值：3
 */
int isAsciiDigit(int x) {
  return 2;
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
/* 中文：conditional(x, y, z) —— 等价于 x ? y : z
 *   示例：conditional(2,4,5) = 4
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：16
 *   分值：3
 */
int conditional(int x, int y, int z) {
  return 2;
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
/* 中文：isLessOrEqual(x, y) —— 若 x <= y 返回 1，否则返回 0
 *   示例：isLessOrEqual(4,5) = 1
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：24
 *   分值：3
 */
int isLessOrEqual(int x, int y) {
  return 2;
}
//4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
/* 中文：logicalNeg(x) —— 用除 ! 以外的全部允许操作符实现逻辑非
 *   示例：logicalNeg(3) = 0，logicalNeg(0) = 1
 *   允许操作符：~ & ^ | + << >>
 *   最多操作符：12
 *   分值：4
 */
int logicalNeg(int x) {
  return 2;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
/* 中文：howManyBits(x) —— 返回用补码表示 x 所需的最少位数
 *   示例：howManyBits(12) = 5
 *         howManyBits(298) = 10
 *         howManyBits(-5) = 4
 *         howManyBits(0)  = 1
 *         howManyBits(-1) = 1
 *         howManyBits(0x80000000) = 32
 *   允许操作符：! ~ & ^ | + << >>
 *   最多操作符：90
 *   分值：4
 */
int howManyBits(int x) {
  return 0;
}
//float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
/* 中文：floatScale2(uf) —— 返回浮点参数 f 的 2*f 的位级等价结果
 *   参数和结果都以 unsigned int 传递，但应把它们解释为单精度浮点值的位级表示。
 *   当参数是 NaN 时，直接返回该参数。
 *   允许操作符：任意整型/无符号操作，包括 ||、&&；也可以使用 if、while
 *   最多操作符：30
 *   分值：4
 */
unsigned floatScale2(unsigned uf) {
  return 2;
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
/* 中文：floatFloat2Int(uf) —— 返回浮点参数 f 的 (int)f 的位级等价结果
 *   参数以 unsigned int 传递，但应解释为单精度浮点值的位级表示。
 *   任何超出范围的情况（包括 NaN 和无穷大）都应返回 0x80000000u。
 *   允许操作符：任意整型/无符号操作，包括 ||、&&；也可以使用 if、while
 *   最多操作符：30
 *   分值：4
 */
int floatFloat2Int(unsigned uf) {
  return 2;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
/* 中文：floatPower2(x) —— 返回 2.0^x（2.0 的 x 次方，x 为任意 32 位整数）的位级等价结果
 *   返回的 unsigned 值应与单精度浮点数 2.0^x 具有完全相同的位表示。
 *   若结果太小、无法表示为非规格化数（denorm），返回 0；若太大，返回 +INF。
 *   允许操作符：任意整型/无符号操作，包括 ||、&&；也可以使用 if、while
 *   最多操作符：30
 *   分值：4
 */
unsigned floatPower2(int x) {
    return 2;
}
