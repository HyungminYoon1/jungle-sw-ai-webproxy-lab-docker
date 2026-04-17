#! /usr/bin/perl -w
use strict;
use Digest::MD5;
#
# port-for-user.pl - 주어진 사용자에 대해 충돌 가능성이 낮은
#     포트 번호 p를 반환합니다. p는 항상 짝수이므로 사용자는
#     proxy와 Tiny 웹 서버 테스트에 p와 p+1을 사용할 수 있습니다.
#     
#     사용법: ./port-for-user.pl [선택적 사용자 이름]
#
my $maxport = 65536;
my $minport = 1024;


# hashname - 인자의 해시값으로 짝수 포트 번호를 계산합니다
sub hashname {
    my $name = shift;
    my $port;
    my $hash = Digest::MD5::md5_hex($name);
    # 마지막 32비트만 사용 => 마지막 8개의 16진수 자리
    $hash = substr($hash, -8);
    $hash = hex($hash);
    $port = $hash % ($maxport - $minport) + $minport;
    $port = $port & 0xfffffffe;
    print "$name: $port\n";
}


# 명령행 인자가 없으면 userid를 해시하고,
# 있으면 명령행 인자들을 해시합니다.
if($#ARGV == -1) {
    my ($username) = getpwuid($<);
    hashname($username);
} else {
    foreach(@ARGV) {
        hashname($_);
    }
}
