#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <sqlite3.h>

#include "commm.h"

// 多线程结构体
struct thread_pri
{
    pthread_t tid;
    int socket;
};

// 服务器log信息
struct logs *server_log;

sqlite3 *db; // 程序返回一个指向数据库的指针.

int pth_socket;

void insertHistory()
{
    // 获取当前时间
    time_t currentTime = time(NULL);
    struct tm *localTime = localtime(&currentTime);

    // 将时间转换为字符串形式
    char dateString[20];
    strftime(dateString, sizeof(dateString), "%Y-%m-%d %H:%M:%S", localTime);
    char sqlcmd[128];                                                                                                           // sql语句
    sprintf(sqlcmd, "insert into history (usr,word,data)values('%s','%s','%s')", server_log->usr, server_log->buf, dateString); // 验证
    char **resultp = (char **)malloc(sizeof(char *) * 10000);                                                                   // 指向结果
    int row, column;                                                                                                            // row 行  column列
    int ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    printf("历史记录已生成\n");
    sqlite3_free_table(resultp);
}

// 注册
void signIn()
{
    char sqlcmd[128];                                                                                    // sql语句
    sprintf(sqlcmd, "INSERT INTO usrs(usr, pwd) VALUES('%s', '%s');", server_log->usr, server_log->pwd); // 添加usr
    char **resultp;                                                                                      // 指向结果
    int row, column;                                                                                     // row 行  column列
    int ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    strcpy(server_log->buf, "添加成功");
    sqlite3_free_table(resultp);
}

// 登录
void enter()
{
    char sqlcmd[128];                                                                                     // sql语句
    sprintf(sqlcmd, "select * from usrs where usr='%s' and pwd='%s';", server_log->usr, server_log->pwd); // 验证
    char **resultp;                                                                                       // 指向结果
    int row, column;                                                                                      // row 行  column列
    int ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    else if (row > 0)
    {
        strcpy(server_log->buf, "用户信息已验证");
        server_log->code = 7;
    }
    else
    {
        strcpy(server_log->buf, "密码错误");
        server_log->code = 0;
    }
    sqlite3_free_table(resultp);
}

// 验证用户名是否存在(初次登录)
void verifyFirst()
{
    char sqlcmd[128];                                                       // sql语句
    sprintf(sqlcmd, "select * from usrs where usr='%s';", server_log->usr); // 查找usr
    char **resultp;                                                         // 指向结果
    int row, column;                                                        // row 行  column列
    int ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    else if (row > 0)
    {
        strcpy(server_log->buf, "用户名已存在，请输入密码");
        server_log->code = 2;
    }
    else
    {
        strcpy(server_log->buf, "用户名不存在,请输入密码注册");
        server_log->code = 1;
    }
    sqlite3_free_table(resultp);
}

// 查单词
int findWord()
{

    FILE *fp;
    char *red = (char *)malloc(sizeof(char) * 10000);
    char cpy[32];
    int flag = 0;

    if ((fp = fopen("./dict.txt", "r")) == NULL)
    {
        perror("fopen");
        return -1;
    }
    printf("正在查单词%s\n", server_log->buf);
    while ((fgets(red, 10000, fp)) != NULL) // 每次读取一行存入red，复制前面的单词部分给cpy对比
    {
        for (int i = 0; i < strlen(server_log->buf); i++)
        {
            cpy[i] = red[i];
        }
        if (strcmp(cpy, server_log->buf) == 0)
        {
            strcpy(server_log->buf, red);
            flag = 1;
            break;
        }
    }
    if (!flag)
    {
        strcpy(server_log->buf, "未在字典中找到此单词");
    }
    free(red);
    fclose(fp);
    return 0;
}

// 修改密码
void updatePwd()
{
    char sqlcmd[128];                                                                              // sql语句
    sprintf(sqlcmd, "update usrs set pwd='%s' where usr='%s';", server_log->buf, server_log->usr); // 修改pwd
    char **resultp;                                                                                // 指向结果
    int row, column;                                                                               // row 行  column列
    int ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    sprintf(sqlcmd, "select * from usrs where usr='%s' and pwd='%s';", server_log->usr, server_log->buf); // 验证
    ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    if (row > 0)
    {
        printf("更新密码成功\n");
    }
    sqlite3_free_table(resultp);
}

void getHistory()
{

    char sqlcmd[128];                                                          // sql语句
    sprintf(sqlcmd, "select * from history where usr='%s';", server_log->usr); // 查找usr
    char **resultp;                                                            // 指向结果
    int row, column;                                                           // row 行  column列
    int ret = sqlite3_get_table(db, sqlcmd, &resultp, &row, &column, NULL);
    if (ret)
    {
        printf("get table err %s\n", sqlite3_errmsg(db));
    }
    strcpy(server_log->buf, "正在查询");

    // 传给二维数组
    for (int i = 0; i < (row + 1) * column; i++)
    {
        printf("%s\n", resultp[i]);
        strcat(server_log->sql, "          ");
        strcat(server_log->sql, resultp[i]); // 将 resultp[i] 拼接到 server_log->sql[i]
        if ((i + 1) % column == 0)
        {
            strcat(server_log->sql, "\n");
            printf("\n");
        } // ：如果需要在每行的末尾添加换行符
        printf("sql:%s\n", server_log->sql);
    }

    sqlite3_free_table(resultp); // 释放结果集内存
}

int switchSev()
{
    switch (server_log->code)
    {
    case 0:
        verifyFirst();
        break;
    case 1:
        signIn();
        server_log->code = 7;
        break;
    case 2:
        enter();
        break;
    case 3:
        server_log->code = 0;
        strcpy(server_log->buf, "请重新登录\n");
        break;
    case 4:
        updatePwd();
        break;
    case 5:
        getHistory();
        break;

    case 6:
        insertHistory();
        findWord();
        break;

    default:
        break;
    }
    return 0;
}

// 多线程回调函数（传入accept的socket赋值给线程socket）
void *serverForClient(void *args)
{
    // 线程socket
    struct thread_pri *pri = args;
    int pth_socket = pri->socket;
    printf("一个新线程已经建立\n");

    // 接受字节大小
    ssize_t bytes_received;

    memset(server_log, 0, sizeof(struct logs)); // 接受前清空

    while (1)
    {

        // 接收客户端发送的数据
        bytes_received = recv(pth_socket, server_log, sizeof(struct logs), 0);
        if (bytes_received < 0)
        {
            perror("bytes_received");
            return NULL;
        }
        else if (bytes_received == 0)
        {

            break;
        }

        printf("服务器接收：%s\n%s\n", server_log->usr, server_log->pwd);

        // 在这里处理接收到的数据
        switchSev();
        if (strcmp(server_log->buf, "") != 0)
        {
            printf("服务器回复:%s\n", server_log->buf);
        }

        send(pth_socket, server_log, sizeof(struct logs), 0);
        if (server_log->code == 5)
        {
            free(server_log->sql);
        }
    }
    printf("客户端已经关闭，关闭线程\n");
    // 关闭连接
    close(pth_socket);
    free(pri);

    return NULL;
}

int main()
{
    server_log = (struct logs *)malloc(sizeof(struct logs));

    // 打开或创建数据库
    if (sqlite3_open("./dic.db", &db))
    {
        printf("open err:%s\n", sqlite3_errmsg(db));
        return -1;
    }

    // 服务器IP
    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(65001);

    // 客户端IP
    struct sockaddr_in peer_addr;
    socklen_t addrlen = sizeof(peer_addr);

    // 创建服务器socket
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
    {
        perror("listen_socket");
        return -1;
    }

    // 绑定
    if (bind(listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        perror("bind");
        return -1;
    }

    // 监听
    if (listen(listen_socket, 20))
    {
        perror("listen");
        return -1;
    }

    // 多线程连接客户端
    while (1)
    {
        printf("服务器正在等待一个新的连接...\n");
        // 接受连接请求

        struct thread_pri *pri = (struct thread_pri *)malloc(sizeof(struct thread_pri));
        pri->socket = accept(listen_socket, (struct sockaddr *)&peer_addr, &addrlen);
        if (pri->socket < 0)
        {
            perror("accept");
            return -1;
        }
        else if (addrlen < sizeof(peer_addr)) // 判断输入IP输入大于预设
        {
            printf("请使用IPv4");
            return -1;
        }
        printf("New client connected: %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

        // 创建新线程
        int ret = pthread_create(&pri->tid, NULL, serverForClient, pri);

        if (ret < 0)
        {
            perror("pthread");
            return -1;
        }

        // 分离线程，使其在退出时能够自动释放资源
        if (pthread_detach(pri->tid) != 0)
        {
            perror("Failed to detach thread");
            exit(1);
        }
    }

    free(server_log);

    if (sqlite3_close(db))
    {
        printf("close err:%s\n", sqlite3_errmsg(db));
        return -45;
    }

    return 0;
}