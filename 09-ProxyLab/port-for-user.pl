#! /usr/bin/perl -w
use strict;
use Digest::MD5;
#
# port-for-user.pl - Return a port number, p, for a given user, with a
#     low probability of collisions. The port p is always even, so that
#     users can use p and p+1 for testing with proxy and the Tiny web
#     server.
#     
#     usage: ./port-for-user.pl [optional user name]
#
# 中文翻译：
# port-for-user.pl —— 为给定用户返回一个端口号 p，发生冲突的概率很低。
# 返回的端口 p 永远是偶数，这样用户可以用 p 和 p+1 分别给代理和 Tiny 服务器测试。
# 用法：./port-for-user.pl [可选用户名]
my $maxport = 65536;
my $minport = 1024;


# hashname - compute an even port number from a hash of the argument
# 中文翻译：
# hashname —— 由参数（用户名）的哈希值计算出一个偶数端口号。
sub hashname {
    my $name = shift;
    my $port;
    my $hash = Digest::MD5::md5_hex($name);
    # take only the last 32 bits => last 8 hex digits
# 中文：只取后 32 位（即后 8 个十六进制数字）
    $hash = substr($hash, -8);
    $hash = hex($hash);
    $port = $hash % ($maxport - $minport) + $minport;
    $port = $port & 0xfffffffe;
    print "$name: $port\n";
}


# If called with no command line arg, then hash the userid, otherwise
# hash the command line argument(s).
# 中文翻译：
# 如果没有任何命令行参数，就对当前用户 ID 取哈希；
# 否则对命令行给出的每个参数取哈希。
if($#ARGV == -1) {
    my ($username) = getpwuid($<);
    hashname($username);
} else {
    foreach(@ARGV) {
        hashname($_);
    }
}
