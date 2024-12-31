# 基于Linux平台的HTTP服务器
[English Version](README_en.md)

一个使用纯C语言开发的linux平台下的http服务器，没有使用任何的框架。只能解析本地的静态文件，对于post方法没有实现。

## 项目概述

下面是文件的概述


```shell
.
├── README.md
├── html_docs # html资源，下面的页面可以随便替换
│   ├── about.html
│   ├── box.css
│   ├── box.html
│   ├── flexbox.css
│   ├── index.css
│   ├── index.html
│   ├── index.js
│   ├── javascript.html
│   ├── not_found.html
│   ├── show.html
│   ├── test.html
│   └── unimplement.html
├── main # 可执行程序
├── main.c # 主函数
└── run.sh # 编译，可以使用make替代
```

## 核心函数说明

因为这个项目很短，是一个非常简单的示例程序。所以将大部分的解释放在了程序里面。

核心的函数如下

```cpp
/// @brief 发起请求
/// @param pthread_sock 线程的句柄，客户端的句柄
/// @return 空指针，线程终止信号
void* do_http_requeset(void* pthread_sock)
{
    int client_sock = *((int*)pthread_sock);
    char read_buf[BUFSIZ] = "\0";
    char head_buf[HEADER_TYPE_LEN][BUFSIZ];
    char path[BUFSIZ] = "\0";
    Server_Reply server_reply;

    int read_len = get_line(client_sock, read_buf, sizeof(read_buf));
    if (read_len > 0)
    {
        printf("has get the head line: %s\n", read_buf);
		
        /*
        浏览器发来的第一行：GET / HTTP/1.1
		*/
        strcat(head_buf[METHOD], strtok(read_buf, " "));
        strcat(head_buf[URL], strtok(NULL, " "));
        strcat(head_buf[PROTOOL], strtok(NULL, " "));

        do
        {
            read_len = get_line(client_sock, read_buf, sizeof(read_buf));
            DEBUG(read_buf);
        } while (read_len > 0);//舍弃掉其他的数据，详细请看输出部分

        if (strncmp(head_buf[METHOD], "GET", 4) == 0)
        {
            DEBUG("it is GET method");
            strcat(path, "./html_docs/");
            strcat(path, head_buf[URL]);
            struct stat st;
            if (stat(path, &st) == -1)
            {
                memset(path, '\0', BUFSIZ);
                strcpy(path, "./html_docs/not_found.html");
                server_reply.signal = 404; // 文件不存在
                strcpy(server_reply.describe, "NOT FOUND");
            } // 文件不存在,跳转到发送请求
            else
            {
                if (S_ISDIR(st.st_mode))
                {
                    strcat(path, "index.html");
                }// 是空的目录，自动映射到index.html
                server_reply.signal = 200;// 请求成功
                strcpy(server_reply.describe, "OK");
            } // 使用默认路径
        }     // get方法
        else
        {
            DEBUG("it is other method\n");
            strcat(path, "./html_docs/unimplement.html");
            server_reply.signal = 501;// 请求未实现
            strcpy(server_reply.describe, "Method Not Implemented");
        } // 其他方法不存在
    }     // 方法正确
    else
    {
        DEBUG("request method error");
        strcat(path, "./html_docs/bad_request.html");
        server_reply.signal = 400;//请求错误
        strcpy(server_reply.describe, "BAD REQUEST");
    }
    do_http_response(client_sock, path, server_reply);

    close(client_sock);
    if (pthread_sock)
    {
        free(pthread_sock);
    } // 要在线程函数内终止
    return NULL;
}
```

运行结果示例：

```shell
$ sudo ./main 
this client has quit! # 建立连接中
has get the head line: GET / HTTP/1.1
Host: localhost
Connection: keep-alive
sec-ch-ua: "Microsoft Edge";v="131", "Chromium";v="131", "Not_A Brand";v="24"
sec-ch-ua-mobile: ?0t Edge";v="131", "Chromium";v="131", "Not_A Brand";v="24"
sec-ch-ua-platform: "Windows"="131", "Chromium";v="131", "Not_A Brand";v="24"
Upgrade-Insecure-Requests: 1"="131", "Chromium";v="131", "Not_A Brand";v="24"
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Site: noneplication/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Mode: navigateation/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-User: ?1vigateation/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Dest: documentation/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate, br, zstdapplication/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6age/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6age/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
it is GET method
HTTP/1.0 200 OK
Server:Moon Server
                  Content-Type:text/html
                  Connection:Close
Content-Length:791


<!DOCTYPE html>

<html>



<head>

    <link rel="stylesheet" type="text/css" href="index.css" />

    <link rel="javascript" type="javascript" href="index.js" />

</head>



<body>



    <ul>

        <li><a class="active" href="#home">Home</a></li>

        <li><a href="show.html">News</a></li>

        <li><a href="javascript.html">Contact</a></li>

        <li><a href="about.html">About</a></li>

    </ul>



    <div style="margin-left:10%;padding:1px 16px;height:1000px;">

        <h1>HomePage</h1>

        <h2>FirstCharpter</h2>

        <a href="box.html">a system about education</a><br/>

        <a href="test.html">a test for system</a>

        <p></p>

        <p></p>

        <p></p>

        <p></p>

        <p></p>

        <p></p>

        <p></p>

        <p></p>

    </div>



</body>



</html>
response has finished!
this client has quit!
has get the head line: GET /index.css HTTP/1.1
Host: localhostHTTP/1.1
Connection: keep-alive1
sec-ch-ua-platform: "Windows"
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
sec-ch-ua: "Microsoft Edge";v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
sec-ch-ua-mobile: ?0t Edge";v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept: text/css,*/*;q=0.1";v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Sec-Fetch-Site: same-origin;v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Sec-Fetch-Mode: no-corsigin;v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Sec-Fetch-Dest: stylersigin;v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Referer: http://localhost/n;v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept-Encoding: gzip, deflate, br, zstdromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
it is GET method
HTTP/1.0 200 OK
Server:Moon Server
                  Content-Type:text/html
                  Connection:Close
Content-Length:1430


body {

    font-family: Arial, sans-serif;

}



.button {

    color: white;

    background-color: blue;

    width: 100px;

    height: 20px;

    padding: auto;

}



header {

    display: flex;

}



header .profile-thumbnail {

    width: 50px;

    height: 50px;

    border-radius: 4px;

}



header .profile-name {

    display: flex;

    margin-left: 10px;

}



header .follow-btn {

    display: flex;

    margin: 0 0 0 auto;



}



header .follow-btn button {

    border: 0;

    border-radius: 3px;

    padding: 5px;

}



header h3,

header h4 {

    display: flex;

    margin: 0;

}



#inner p {

    margin-bottom: 10px;

    font-size: 20px;

}



#inner hr {

    margin: 20px 0;

    border-style: solid;

    opacity: 0.1;

}



footer {

    display: flex;

}



footer .stats {

    display: flex;

    font-size: 15px;

}



footer .stats strong {

    font-size: 18px;

}



footer .stats .likes {

    margin-left: 10px;

}



footer .cta {

    margin-left: auto;

}



footer .cta button {

    border: 0;

    background: transparent;

}



body {

    margin: 0;

}



ul {

    list-style-type: none;

    margin: 0;

    padding: 0;

    width: 10%;

    background-color: #f1f1f1;

    position: fixed;

    height: 100%;

    overflow: auto;

}



li a {

    display: block;

    color: #000;

    padding: 8px 16px;

    text-decoration: none;

}



li a.active {

    background-color: #4CAF50;

    color: white;

}



li a:hover:not(.active) {

    background-color: #555;

    color: white;

}
response has finished!
this client has quit!
has get the head line: GET /favicon.ico HTTP/1.1
Host: localhosto
Connection: keep-alive1.1
sec-ch-ua-platform: "Windows"
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
sec-ch-ua: "Microsoft Edge";v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
sec-ch-ua-mobile: ?0t Edge";v="131", "Chromium";v="131", "Not_A Brand";v="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Sec-Fetch-Site: same-originbp,image/apng,image/svg+xml,image/*,*/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Sec-Fetch-Mode: no-corsiginbp,image/apng,image/svg+xml,image/*,*/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Sec-Fetch-Dest: imagersiginbp,image/apng,image/svg+xml,image/*,*/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Referer: http://localhost/nbp,image/apng,image/svg+xml,image/*,*/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept-Encoding: gzip, deflate, br, zstd,image/svg+xml,image/*,*/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6/*;q=0.8="24"ML, like Gecko) Chrome/131.0.0.0 Safari/537.36 Edg/131.0.0.0
it is GET method
HTTP/1.0 404 NOT FOUND
Server:Moon Server
                  Content-Type:text/html
                  Connection:Close
Content-Length:189


<html>

    <header>

        <title>NOT FOUND</title>

    </header>

    <body>

        <h1>404 NOT FOUND</h1>

        <p>you request page is missing, please try again</p>

    </body>

</html>
response has finished!
^C
Server Stop! Please check the reason
: Success
```

## 许可证说明

本项目基于`MIT`许可证，详情见`LICENSE`文件

项目地址(https://github.com/doctordragon666/http-server.git)