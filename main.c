#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 80
#define IP "192.168.170.128"
const int debug = 1;

enum HEADER
{
    METHOD,
    URL,
    PROTOOL,
    HEADER_TYPE_LEN
}; // 第一个是请求方法，第二个URL，最后是协议版本

/// @brief 响应结构体
typedef struct
{
    int signal;        // 响应代号
    char describe[32]; // 描述
} Server_Reply;

/// @brief 打印错误信息
/// @param tip 错误信息
void BUG(const char *tip)
{
    if (tip == NULL)
        return;
    if (debug)
    {
        fprintf(stderr, tip, strlen(tip));
        printf("\n");
    }
}

/// @brief sock出错处理
/// @param ret
/// @param method 错误的方法名字
void SOCKET_ERROR(int ret, const char *method)
{
    if (ret < 0)
    {
        perror(method);
        exit(1);
    }
}

/// @brief 获取一行信息
/// @param sockfd 服务器句柄
/// @param buf 读取容器
/// @param size 读取大小
/// @return 读取的长度
int get_line(int sockfd, char *buf, int size)
{
    int count = 0;
    char c;
    int len = 0;
    while (count < size - 1)
    {
        len = read(sockfd, &c, sizeof(char));
        if (len > 0)
        {
            if (c == '\r')
            {
                continue;
            }
            else if (c == '\n')
            {
                break;
            } // 读到http的换行符
            buf[count] = c;
            count++;
        } // 读取正常
        else if (len == -1)
        {
            perror("read failed");
            count = -1;
            break;
        } // 读取有误
        else
        {
            BUG("client close. \n");
        }
    }
    return count;
}

/// @brief 发送服务器标识头部
/// @param client_sock 客户端句柄
/// @param file_id 文件句柄
/// @param server_reply 服务器响应
/// @return 成功0失败-1
int header(int client_sock, FILE *file_id, Server_Reply server_reply)
{
    char buf[BUFSIZ] = "\0";

    // 粘接头部信息
    char tmp[64];
    snprintf(tmp, 64, "HTTP/1.0 %d %s\r\n", server_reply.signal, server_reply.describe);
    strcat(buf, tmp);
    strcat(buf, "Server:Moon Server\r\n \
                 Content-Type:text/html\r\n \
                 Connection:Close\r\n"); // 浏览器提示

    // 获取html文件大小
    int fileid = fileno(file_id);
    struct stat st;
    SOCKET_ERROR(fstat(fileid, &st), "fstat");
    snprintf(tmp, 64, "Content-Length:%ld\r\n\r\n", st.st_size);
    strcat(buf, tmp);
    BUG(buf);

    if (send(client_sock, buf, strlen(buf), 0) < 0)
    {
        fprintf(stderr, "send failed.data:%s,reason:%s\n", buf, strerror(errno));
        return -1;
    } // 发送头部给浏览器

    return 0;
} // 下一步交给page函数

/// @brief 写入html文件内容
/// @param client_sock 客户端句柄
/// @param file_id 文件句柄
void page(int client_sock, FILE *file_id)
{
    char buf[1024];
    fgets(buf, sizeof(buf), file_id);

    while (!feof(file_id))
    {
        if (buf == NULL)
            return; // 文件为空
        SOCKET_ERROR(write(client_sock, buf, strlen(buf)), "write");
        //BUG(buf);
        fgets(buf, sizeof(buf), file_id);
    } // 文件到达末尾
} // 发送完页面结束响应过程

/// @brief 响应请求
/// @param client_sock 客户端句柄
/// @param path 文件路径
/// @param server_reply 响应请求
void do_http_response(int client_sock, char *path, Server_Reply server_reply)
{
    FILE *resource = fopen(path, "r");
    if (resource == NULL)
    {
        perror("open file error");
        return;
    }

    SOCKET_ERROR(header(client_sock, resource, server_reply), "header");
    page(client_sock, resource);

    fclose(resource);
    BUG("response has finished!");
}

/// @brief 发起请求
/// @param pthread_sock 线程的句柄，就是多个句柄
/// @return 空指针，线程终止信号
void *do_http_requeset(void *pthread_sock)
{
    int client_sock = *((int *)pthread_sock);
    char read_buf[BUFSIZ] = "\0";
    char head_buf[HEADER_TYPE_LEN][BUFSIZ];
    char path[BUFSIZ] = "\0";
    Server_Reply server_reply;

    int read_len = get_line(client_sock, read_buf, sizeof(read_buf));
    if (read_len > 0)
    {
        printf("has get the head line: %s\n", read_buf);

        strcat(head_buf[METHOD],strtok(read_buf," "));
        strcat(head_buf[URL],strtok(NULL," "));
        strcat(head_buf[PROTOOL],strtok(NULL," "));

        do
        {
            read_len = get_line(client_sock, read_buf, sizeof(read_buf));
        } while (read_len > 0);

        if (strncmp(head_buf[METHOD], "GET", 4) == 0)
        {
            BUG("it is GET method");
            strcat(path, "./html_docs/");
            strcat(path, head_buf[1]);
            struct stat st;
            if (stat(path, &st) == -1)
            {
                memset(path, '\0', BUFSIZ);
                strcpy(path, "./html_docs/not_found.html");
                server_reply.signal = 404;
                strcpy(server_reply.describe, "NOT FOUND");
            }//文件不存在,跳转到发送请求
            else
            {
                if (S_ISDIR(st.st_mode))
                {
                    strcat(path, "index.html");
                }
                server_reply.signal = 200;
                strcpy(server_reply.describe, "OK");
            }//使用默认路径
        }// get方法
        else
        {
            BUG("it is other method\n");
            strcat(path, "./html_docs/unimplement.html");
            server_reply.signal = 501;
            strcpy(server_reply.describe, "Method Not Implemented");
        }//其他方法不存在
    }//方法正确
    else
    {
        BUG("request method error");
        strcat(path, "./html_docs/bad_request.html");
        server_reply.signal = 400;
        strcpy(server_reply.describe, "BAD REQUEST");
    }
    do_http_response(client_sock, path, server_reply);

    close(client_sock);
    if (pthread_sock)
    {
        free(pthread_sock);
    } // 要在线程函数内终止，否则会发生错误
    return NULL;
}

int main()
{
    pthread_t thread_id;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 定义地址，协议和端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    SOCKET_ERROR(bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)), "bind");

    SOCKET_ERROR(listen(sock_fd, 128), "listen");

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        SOCKET_ERROR(client_fd, "accept");

        int *pclient_sock = (int *)malloc(sizeof(int));
        *pclient_sock = client_fd;
        if (pthread_create(&thread_id, NULL, do_http_requeset, (void *)pclient_sock) != 0)
        {
            fprintf(stderr, "pthread error, reason:%s", strerror(errno));
            return -1;
        }

        printf("this client has quit!\n");
    }

    close(sock_fd);
    return 0;
}
