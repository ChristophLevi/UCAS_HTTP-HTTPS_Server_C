#ifndef basefun_h
#define basefun_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// read方法需要的头文件
#include <unistd.h>
// socket方法需要的头文件
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
// htonl 方法需要的头文件
#include <netinet/in.h>
// inet_ntop方法需要的头文件
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// 定义Socket函数
int Socket(int family, int type, int protocol);
// 定义Bind函数
void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
// 定义Listen函数
void Listen(int fd, int backlog);
// 定义Accept函数
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
// 定义Connect函数
void Connect(int fd, const struct sockaddr *sa, socklen_t salen);
// 定义Close函数
void Close(int fd);
// 定义Write函数
void Write(int fd, void *ptr, size_t nbytes);
// 定义Read函数
long Read(int fd, void *ptr, size_t nbytes);
// 定义SSL_Read函数
long SSL_Read(SSL *ssl, void *buf, size_t count);
// 定义SSL_Write函数
void SSL_Write(SSL *ssl, void *buf, size_t count);
// 定义find_url函数
void find_url(char *url, char *filename);
// 定义get_filetype函数
void get_filetype(char *filename, char *filetype);
// 定义https_response函数
void https_response(SSL *ssl, int connfd, char *buf, char *url);
// 定义load_SSL函数
SSL *load_SSL(int fd);
#endif
