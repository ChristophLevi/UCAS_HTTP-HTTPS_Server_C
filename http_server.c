#include "basefun.h"
#include <ctype.h>
#include <pthread.h>

#define MAXLINE 1 << 20
#define SERV_PORT1 80
#define SERV_PORT2 443
#define BACKLOGSIZE 20

void http_server()
{
    printf("http..............\n");
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE], first_line[MAXLINE], left_line[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char str[INET_ADDRSTRLEN];
    char filename[MAXLINE];
    long n;
    int on = 1;
    pid_t pid;
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT1);
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(listenfd, BACKLOGSIZE);
    while (1)
    {
        cliaddr_len = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        pid = fork();
        if (pid < 0)
        {
            printf("fork error");
        }
        else if (pid == 0)
        { 
            while (1)
            {
                n = Read(connfd, buf, MAXLINE);
                if (n == 0)
                {
                    printf("the client has been closed.\n");
                    break;
                }
                printf("%s\n", buf);
                sscanf(buf, "%s %s %s", method, url, version);
                printf("method = %s\n", method);
                printf("url = %s\n", url);
                printf("version = %s\n", version);

                if (strcasecmp(method, "GET") == 0 && strstr(version, "HTTP"))
                {
                    struct sockaddr_in addr;
                    socklen_t addr_size = sizeof(struct sockaddr_in);
                    getsockname(connfd, (struct sockaddr *)&addr, &addr_size);
                    char server_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &addr.sin_addr, server_ip, INET_ADDRSTRLEN);
                    printf("host ip is = %s\n", server_ip);
                    if (strcasecmp(method, "GET") == 0 && strstr(version, "HTTP"))
                    {
                        char response[2048]; 
                        snprintf(response, sizeof(response),
                                 "HTTP/1.1 301 Moved Permanently\r\n"
                                 "Location: https://%s%s\r\n",
                                 server_ip, url);
                        printf("Response headers:\n%s", response);
                        send(connfd, response, strlen(response), 0);
                        Close(connfd); 
                        break;
                    }
                }
            }
        }
        else
        { 
            Close(connfd);
        }
    }
}

void https_server()
{
    printf("https..............\n");
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE], first_line[MAXLINE], left_line[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char str[INET_ADDRSTRLEN];
    char filename[MAXLINE];
    long n;
    int i, pid;
    int on = 1;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    // 初始化myaddr参数
    bzero(&servaddr, sizeof(servaddr)); // 结构体清零
    // 对servaddr 结构体进行赋值
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT2);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(listenfd, BACKLOGSIZE);

    // 死循环中进行accept()
    while (1)
    {
        cliaddr_len = sizeof(cliaddr);

        // accept()函数返回一个connfd描述符
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        // 初始化SSL会话
        SSL *ssl = load_SSL(connfd);
        pid = fork();
        if (pid < 0)
        {
            printf("fork error");
        }
        else if (pid == 0)
        { // pid=0表示子进程
            while (1)
            {

                // 进行SSL握手，确保SSL连接安全建立
                if (SSL_accept(ssl) == -1)
                {
                    ERR_print_errors_fp(stderr);
                }
                // 从SSL连接中读取数据
                n = SSL_Read(ssl, buf, MAXLINE);
                if (n == 0)
                {
                    printf("the client has been closed.\n");
                    break;
                }
                printf("%s\n", buf);
                sscanf(buf, "%s %s %s", method, url, version);
                printf("method = %s\n", method);
                printf("url = %s\n", url);
                printf("version = %s\n", version);

                if (strcasecmp(method, "GET") == 0)
                {
                    https_response(ssl, connfd, buf, url); // 响应客户端
                }
            }
            close(connfd);
            SSL_free(ssl);
        }

        else
        { // pid>0表示父进程
            close(connfd);
            SSL_free(ssl);
        }
    }
}

int main(int argc, char* argv[])
{
    pid_t pid1, pid2;
    // 创建第一个子进程
    pid1 = fork();
    if (pid1 < 0) // 检查fork是否失败
        printf("fork1 error\n");
    if (pid1 == 0) // 在子进程中运行HTTP服务器
        http_server();
    // 创建第二个子进程
    pid2 = fork();
    if (pid2 < 0) // 检查fork是否失败
        printf("fork2 error\n");
    if (pid2 == 0) // 在子进程中运行HTTPS服务器
        https_server();
    int st1, st2;
    waitpid(pid1, &st1, 0); // 等待第一个子进程结束
    waitpid(pid2, &st2, 0); // 等待第二个子进程结束
    return 0;
}

