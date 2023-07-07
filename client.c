#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "commm.h"

// 客户端信息
struct logs *client_log;

int cl_socket; // 客户端套接字

// 主界面
void displayMain()
{

    memset(client_log->buf, 0, sizeof(client_log->buf));
    printf("------个人电子词典-----\n");
    printf("-------6.查单词-------\n");
    printf("-------4.修改密码------\n");
    printf("-------5.历史记录-----\n");
    printf("-------8.退出程序-----\n");
    printf("-------3.退出登录-----\n");
    printf("输入功能对应的数字\n");
    scanf("%d", &client_log->code);
    getchar();
    if (client_log->code == 6)
    {
        printf("输入你想要查询的单词\n");
        // fgets(client_log->buf, sizeof(client_log->buf), stdin);
        scanf("%s", client_log->buf);
    }
    if (client_log->code == 4)
    {
        printf("输入你的新密码\n");
        // fgets(client_log->buf, sizeof(client_log->buf), stdin);
        scanf("%s", client_log->buf);
    }
}

// 内部响应界面
void displaySuccess()
{

    if (client_log->code == 4)
    {
        printf("新的密码为:%s\n", client_log->buf);
    }
    else if (client_log->code == 5)
    {
        printf("您的查询历史如下:\n");

        for (int i = 0; i < strlen(client_log->sql); i++)
        {
            printf("%c", client_log->sql[i]);
        }
    }

    printf("按任意键退回主界面\n");
    getchar();
    getchar();
}

// 首次登录
int logFirst()
{
    // 验证登录名

    printf("-----登录-----\n");
    printf("-----用户名---\n");
    scanf("%s", client_log->usr);
    printf("您输入的用户名为：%s\n", client_log->usr);
    printf("已发送登录请求\n");

    return 0;
}

int switchFun()
{
    switch (client_log->code)
    {
    case 0:
        logFirst();
        break;
    case 1:
        printf("请输入密码：");
        scanf("%s", client_log->pwd);
        break;
    case 2:
        printf("请输入密码：");
        scanf("%s", client_log->pwd);
        break;
    case 3:
        logFirst();
        break;
    case 4:
        displaySuccess();
        displayMain();
        break;
    case 5:
        displaySuccess();
        displayMain();
        break;

    case 6:
        displaySuccess();
        displayMain();
        break;

    case 7:
        displayMain();
        break;
    case 8:
        exit(0);
        break;

    default:
        break;
    }
    return 0;
}

int main()
{
    client_log = (struct logs *)malloc(sizeof(struct logs));

    memset(client_log, 0, sizeof(struct logs)); // 初始化

    //  服务器地址
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("192.168.44.130");
    serv_addr.sin_port = htons(65001);

    // 服务器配置
    cl_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cl_socket < 0)
    {
        perror("cl_socket");
        return -1;
    }

    // 连接服务器
    if (connect(cl_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        perror("connect err");
        return -1;
    }
    while (1)
    {
        switchFun();
        if (send(cl_socket, client_log, sizeof(struct logs), 0) < 0)
        {
            perror("send err");
            return -1;
        }

        int ret = recv(cl_socket, client_log, sizeof(struct logs), 0);
        if (ret < 0)
        {
            perror("recv err");
            return -1;
        }
        else if (ret == 0)
        {
            break;
        }
        else if (strcmp(client_log->buf, "") != 0)
        {

            printf("收到服务器回复:%s\n", client_log->buf);
        }
    }

    printf("服务器已经关闭，请求失败\n");
    close(cl_socket);

    free(client_log);
    return 0;
}