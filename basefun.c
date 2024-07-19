#include "basefun.h"
#define MAXLINE 1 << 20

int Socket(int family, int type, int protocol)
{
    int sockfd;
    if ((sockfd = socket(family, type, protocol)) < 0)
    {
        perror("socket error");
        exit(1);
    }
    return sockfd;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0)
    {
        perror("bind error");
        exit(1);
    }
}

void Listen(int sockfd, int backlog)
{
    if (listen(sockfd, backlog) < 0)
    {
        perror("listen error");
        exit(1);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd;
    if ((connfd = accept(sockfd, addr, addrlen)) < 0)
    {
        perror("accept error");
        exit(1);
    }
    return connfd;
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (connect(sockfd, addr, addrlen) < 0)
    {
        perror("connect error");
        exit(1);
    }
}

long Read(int fd, void *buf, size_t count)
{
    long n;
    if ((n = read(fd, buf, count)) < 0)
    {
        perror("read error");
        exit(1);
    }
    return n;
}

void Write(int fd, void *buf, size_t count)
{
    if (write(fd, buf, count) < 0)
    {
        perror("write error");
        exit(1);
    }
}

long SSL_Read(SSL *ssl, void *buf, size_t count)
{
    long n;
    if ((n = SSL_read(ssl, buf, count)) < 0)
    {
        perror("read error");
        exit(1);
    }
    return n;
}

void SSL_Write(SSL *ssl, void *buf, size_t count)
{
    if (SSL_write(ssl, buf, count) < 0)
    {
        perror("write error");
        exit(1);
    }
}

void Close(int fd)
{
    if (close(fd) < 0)
    {
        perror("close error");
        exit(1);
    }
}

SSL *load_SSL(int fd)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    printf("Load Certificate...\n");
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method()); 
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_use_certificate_file(ctx, "./keys/cnlab.cert", SSL_FILETYPE_PEM) <= 0)
    {
        printf("load public key error");
        exit(1);
    }
    printf("Load Private Key...\n");
    if (SSL_CTX_use_PrivateKey_file(ctx, "./keys/cnlab.prikey", SSL_FILETYPE_PEM) <= 0)
    {
        printf("load private key error");
        exit(1);
    }
    printf("Verify Private Key...\n");
    if (SSL_CTX_check_private_key(ctx) <= 0)
    {
        printf("check private key error");
        exit(1);
    }
    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        printf("SSL_new error");
        exit(1);
    }
    if (SSL_set_fd(ssl, fd) == 0)
    {
        printf("SSL_set_fd error");
        exit(1);
    }
    return ssl;
}

void find_url(char *url, char *filename)
{
    char *ptr;
    if (!strstr(url, "cgi-bin"))
    {
        sscanf(url, "/%s", filename);
    }
}

void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html") || strstr(filename, ".php"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "video/mp4");
    else
        strcpy(filetype, "text/plain");
}

void https_response(SSL *ssl, int connfd, char *buf, char *url)
{
    struct stat sbuf; 
    int fd;
    char *srcp;
    char response[MAXLINE], filename[MAXLINE], filetype[20];

    find_url(url, filename);
    printf("filename = %s\n", filename);

    if (stat(filename, &sbuf) < 0)
    {
        sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        SSL_write(ssl, response, strlen(response));
        printf("can not find file\n");
        exit(1);
    }
    else
    {
        get_filetype(filename, filetype); 
        fd = open(filename, O_RDONLY);
        char *range_header = strstr(buf, "Range: bytes="); 
        long start = 0, end = sbuf.st_size - 1;
        if (range_header)
        {
            sscanf(range_header, "Range: bytes=%ld-%ld", &start, &end);
            end = (end > sbuf.st_size - 1) ? sbuf.st_size - 1 : end; 
        }
        long send_length = end - start + 1;
        int response_length = 0;
        if (range_header)
        {
            response_length += snprintf(response + response_length, sizeof(response) - response_length, "HTTP/1.1 206 Partial Content\r\n");
            response_length += snprintf(response + response_length, sizeof(response) - response_length, "Content-Range: bytes %ld-%ld/%ld\r\n", start, end, sbuf.st_size);
        }
        else
        {
            response_length += snprintf(response + response_length, sizeof(response) - response_length, "HTTP/1.1 200 OK\r\n");
        }
        response_length += snprintf(response + response_length, sizeof(response) - response_length, "Server: Tiny Web Server\r\n");
        response_length += snprintf(response + response_length, sizeof(response) - response_length, "Content-Length: %ld\r\n", send_length);
        response_length += snprintf(response + response_length, sizeof(response) - response_length, "Content-Type: %s\r\n\r\n", filetype);
        printf("Response headers:\n%s", response);
        SSL_Write(ssl, response, response_length);
        if (range_header)
        {
            lseek(fd, start, SEEK_SET);
            char buffer[MAXLINE];
            long to_read = send_length;
            while (to_read > 0)
            {
                int numread = read(fd, buffer, sizeof(buffer));
                if (numread <= 0)
                    break;
                if (numread > to_read)
                    numread = to_read;
                SSL_write(ssl, buffer, numread);
                to_read -= numread;
            }
        }
        else
        {
            void *srcp = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            SSL_write(ssl, srcp, sbuf.st_size);
            munmap(srcp, sbuf.st_size);
        }
        Close(fd);
    }
}
