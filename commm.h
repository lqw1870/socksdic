#ifndef COMMM_H
#define COMMM_H

// 登录信息
struct logs
{
    char sql[10000];
    char usr[10];
    char pwd[100];
    // 客户端和服务器的信息交互
    char buf[10000];
    int code; // 0-初次登录 1-注册  2-登录  3-退出登录  6-find word 4-history 5-modify passwd 7-登录成功状态  8-退出程序
};

#endif