/***************************************************************************
 * Dr. Evil's Insidious Bomb, Version 1.1
 * Copyright 2011, Dr. Evil Incorporated. All rights reserved.
 *
 * LICENSE:
 *
 * Dr. Evil Incorporated (the PERPETRATOR) hereby grants you (the
 * VICTIM) explicit permission to use this bomb (the BOMB).  This is a
 * time limited license, which expires on the death of the VICTIM.
 * The PERPETRATOR takes no responsibility for damage, frustration,
 * insanity, bug-eyes, carpal-tunnel syndrome, loss of sleep, or other
 * harm to the VICTIM.  Unless the PERPETRATOR wants to take credit,
 * that is.  The VICTIM may not distribute this bomb source code to
 * any enemies of the PERPETRATOR.  No VICTIM may debug,
 * reverse-engineer, run "strings" on, decompile, decrypt, or use any
 * other technique to gain knowledge of and defuse the BOMB.  BOMB
 * proof clothing may not be worn when handling this program.  The
 * PERPETRATOR will not apologize for the PERPETRATOR's poor sense of
 * humor.  This license is null and void where the BOMB is prohibited
 * by law.
 * 中文（许可证翻译，Dr. Evil 的恶趣味许可证）：
 *   Dr. Evil 公司（实施者）在此授予你（受害者）使用本炸弹（BOMB）的明确许可。
 *   这是限时许可，在你（受害者）死亡时到期。
 *   实施者对给受害者造成的伤害、挫败、精神失常、对眼、腕管综合征、失眠等
 *   一切损害概不负责，除非实施者想邀功。
 *   受害者不得把本炸弹源码分发给实施者的任何敌人。
 *   任何受害者不得调试、逆向、对其运行 strings、反编译、解密，
 *   或用任何其他手段了解并拆除炸弹。
 *   处理本程序时不得穿防弹衣。实施者不会为自己差劲的幽默感道歉。
 *   在法律禁止炸弹的地方，本许可无效。
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "support.h"
#include "phases.h"

/* 
 * Note to self: Remember to erase this file so my victims will have no
 * idea what is going on, and so they will all blow up in a
 * spectaculary fiendish explosion. -- Dr. Evil
 * 中文：给自己的备忘——记得删掉这个文件，这样受害者就完全不知道发生了什么，
 * 让他们在一场壮观而恶毒的爆炸中统统升天。—— Dr. Evil
 */

FILE *infile;

int main(int argc, char *argv[])
{
    char *input;

    /* Note to self: remember to port this bomb to Windows and put a
     * fantastic GUI on it. */
    /* 中文：给自己的备忘——记得把炸弹移植到 Windows，再给它加个炫酷的图形界面。 */

    /* When run with no arguments, the bomb reads its input lines
     * from standard input. */
    /* 中文：不带参数运行时，炸弹从标准输入逐行读取输入。 */
    if (argc == 1) {  
	infile = stdin;
    } 

    /* When run with one argument <file>, the bomb reads from <file>
     * until EOF, and then switches to standard input. Thus, as you
     * defuse each phase, you can add its defusing string to <file> and
     * avoid having to retype it. */
    /* 中文：带一个参数 <file> 运行时，炸弹先从该文件读输入，读到文件尾（EOF）
     * 后再转到标准输入。这样每拆掉一关，把该关答案追加进 <file>，
     * 就不用重敲前面已经拆过的答案了。 */
    else if (argc == 2) {
	if (!(infile = fopen(argv[1], "r"))) {
	    printf("%s: Error: Couldn't open %s\n", argv[0], argv[1]);
	    exit(8);
	}
    }

    /* You can't call the bomb with more than 1 command line argument. */
    /* 中文：炸弹最多只接受一个命令行参数，多了不行。 */
    else {
	printf("Usage: %s [<input_file>]\n", argv[0]);
	exit(8);
    }

    /* Do all sorts of secret stuff that makes the bomb harder to defuse. */
    /* 中文：做各种让炸弹更难拆除的秘密手脚。 */
    initialize_bomb();

    printf("Welcome to my fiendish little bomb. You have 6 phases with\n");
    printf("which to blow yourself up. Have a nice day!\n");

    /* Hmm...  Six phases must be more secure than one phase! */
    /* 中文：嗯……六个关卡总该比一个关卡更安全吧！ */
    input = read_line();             /* Get input                   */
    phase_1(input);                  /* Run the phase               */
    phase_defused();                 /* Drat!  They figured it out!
				      * Let me know how they did it. */
    /* 中文：该死！他们居然解出来了！——得搞清楚他们是怎么做到的。 */
    printf("Phase 1 defused. How about the next one?\n");

    /* The second phase is harder.  No one will ever figure out
     * how to defuse this... */
    /* 中文：第二关更难了。没人能解出这个…… */
    input = read_line();
    phase_2(input);
    phase_defused();
    printf("That's number 2.  Keep going!\n");

    /* I guess this is too easy so far.  Some more complex code will
     * confuse people. */
    /* 中文：到目前为止大概太简单了。来点更复杂的代码迷惑一下大家。 */
    input = read_line();
    phase_3(input);
    phase_defused();
    printf("Halfway there!\n");

    /* Oh yeah?  Well, how good is your math?  Try on this saucy problem! */
    /* 中文：哦是吗？那你的数学怎么样？试试这道火辣的问题吧！ */
    input = read_line();
    phase_4(input);
    phase_defused();
    printf("So you got that one.  Try this one.\n");
    
    /* Round and 'round in memory we go, where we stop, the bomb blows! */
    /* 中文：我们在内存里一圈又一圈地转，停在哪里，炸弹就炸在哪里！ */
    input = read_line();
    phase_5(input);
    phase_defused();
    printf("Good work!  On to the next...\n");

    /* This phase will never be used, since no one will get past the
     * earlier ones.  But just in case, make this one extra hard. */
    /* 中文：这一关本不该被用到，因为没人能通过前面那些关。但以防万一，
     * 把这一关做得特别难。 */
    input = read_line();
    phase_6(input);
    phase_defused();

    /* Wow, they got it!  But isn't something... missing?  Perhaps
     * something they overlooked?  Mua ha ha ha ha! */
    /* 中文：哇，他们居然全解出来了！但是不是……还缺了点什么？
     * 也许是他们漏掉的什么东西？哇哈哈哈哈哈！ */

    return 0;
}
