#include<iostream>
using namespace std;
int func(int rdi,int rsi,int rdx,int rcx){
    int rax = rdx;
    rax -= rsi;
    rcx = rax;
    rcx = (rcx >=0 ? 0 : 1);
    rax += rcx;
    rax >>= 1;
    rcx = rax + rsi;
    if(rcx-rdi>0){
        rdx = rcx - 1;
        func(rdi,rsi,rdx,rcx);
        rax *= 2;
        return rax;
    }
    if(rcx-rdi<=0){
        rax = 0;
        if(rcx-rdi>=0){
            return rax;
        }
        else{
            rsi = rcx + 1;
            func(rdi,rsi,rdx,rcx);
            rax = 1 + rax + rax;
            return rax;
        }
    }
}
int main(){
    int rdi, rsi = 0, rdx = 15, rcx = 0;
    cin >> rdi;
}
// 最后需要让rax=0.rcx初始值为0
// phase4初始传参为x<=15,0   func传参:rdi = 初始1参,rsi = 0，rdx = 15,rcx = 0