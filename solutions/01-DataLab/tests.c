/* Testing Code */
/* 中文：测试代码 —— 下面的 test_* 函数是各谜题的参考实现（标准答案）。
 * btest 把你的实现与它们比较，从而判断正确性。 */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */
/* 中文：浮点测试代码用到的辅助例程 */

/* Convert from bit level representation to floating point number */
/* 中文：把位级表示（unsigned）转换为浮点数 */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
/* 中文：把浮点数转换为位级表示（unsigned） */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

//1
int test_bitXor(int x, int y)
{
  return x^y;
}
int test_tmin(void) {
  return 0x80000000;
}
//2
int test_isTmax(int x) {
    return x == 0x7FFFFFFF;
}
int test_allOddBits(int x) {
  int i;
  for (i = 1; i < 32; i+=2)
      if ((x & (1<<i)) == 0)
   return 0;
  return 1;
}







int test_negate(int x) {
  return -x;
}
//3


int test_isAsciiDigit(int x) {
  return (0x30 <= x) && (x <= 0x39);
}
int test_conditional(int x, int y, int z)
{
  return x?y:z;
}
int test_isLessOrEqual(int x, int y)
{
  return x <= y;
}
//4
int test_logicalNeg(int x)
{
  return !x;
}
int test_howManyBits(int x) {
    unsigned int a, cnt;
    x = x<0 ? -x-1 : x;
    a = (unsigned int)x;
    for (cnt=0; a; a>>=1, cnt++)
        ;

    return (int)(cnt + 1);
}
//float
unsigned test_floatScale2(unsigned uf) {
  float f = u2f(uf);
  float tf = 2*f;
  if (isnan(f))
    return uf;
  else
    return f2u(tf);
}
int test_floatFloat2Int(unsigned uf) {
  float f = u2f(uf);
  int x = (int) f;
  return x;
}
unsigned test_floatPower2(int x) {
  float result = 1.0;
  float p2 = 2.0;
  int recip = (x < 0);
  /* treat tmin specially */
  if ((unsigned)x == 0x80000000) {
      return 0;
  }
  if (recip) {
    x = -x;
    p2 = 0.5;
  }
  while (x > 0) {
    if (x & 0x1)
      result = result * p2;
    p2 = p2 * p2;
    x >>= 1;
  }
  return f2u(result);
}
