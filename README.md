# http服务器概述

因为客户端已经成为了浏览器，理论上只需要设计服务器即可。分为以下几部分来实现

1.接受http请求

读取请求行(对http请求要有了解)

2.解析请求

GET请求: 获取url，然后返回url里面的文件

其他请求: 返回501界面

格式出错返回bad request

3.响应请求：上面已经提到

额外操作：

读取文件和并发处理

### http头部分析

分为四行，都以`\r\n`结束

##### 客户端

请求行：包括`请求方法`，`url`，`协议版本`

请求头部：字段名`:`值

空行：

请求的数据：

##### 服务器端

状态行：`协议版本`，`响应代号`，`代号描述`

消息报头：字段名`:`值

空行：

响应正文：

##### 常见响应代号

|                                            | 响应代号 | 代号描述               |
| ------------------------------------------ | -------- | ---------------------- |
| 服务器上存在请求的内容，并可以响应给客户端 | 200      | OK                     |
| 客户端的请求有异常，方法有问题             | 501      | Method Not Implemented |
| 服务器收到请求后，因为自生的问题没法响应   | 500      | Internal Server Error  |
| 请求的内容不存在                           | 404      | NOT FOUND              |
| 客户端发送的请求格式有问题等               | 400      | BAD REQUEST            |

### stat函数

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
int stat(const char *path, struct stat *buf);//文件的名字状态
int fstat(int fd, struct stat *buf);//文件句柄获取状态
int lstat(const char* path, struct stat *buf);//不常用
/*
path:
	文件的路径
buf:
	传入的保存文件状态的指针，用于保存文件的状态
返回值：
	成功返回0，失败返回-1，设置errno
*/
struct stat {
               dev_t     st_dev;     /*  文件的设备编号 */
               ino_t     st_ino;     /* inode number */
               mode_t    st_mode;    /* S_ISREG(st_mode)  是一个普通文件  S_ISDIR(st_mode)  是一个目录*/
               nlink_t   st_nlink;   /* 连接到该文件的硬连接数目，刚建立的文件值为1 */
               uid_t     st_uid;     /* 文件所有者的用户识别码 */
               gid_t     st_gid;     /* 文件所有者的组识别码 */
               dev_t     st_rdev;    /* 若此文件为装置设备文件, 则为其设备编号 */
               off_t     st_size;    /* 总大小，单位b */
               blksize_t st_blksize; /* 文件系统的I/O 缓冲区大小. */
               blkcnt_t  st_blocks;  /* 占用文件区块的个数, 每一区块大小为512 个字节 */
               time_t    st_atime;   /* 最后一次访问的时间 */
               time_t    st_mtime;   /* 最后一次修改的时间*/
               time_t    st_ctime;   /* 最近一次被更改的时间 */
           };
```

# 实战

### 处理头部

| 请求方法 | 空格 | URL  | 空格 | 协议版本 | \r\n |
| -------- | ---- | ---- | ---- | -------- | ---- |
| 字段名   | :    | 值   | \r\n |          |      |
| \r\n     |      |      |      |          |      |
| data     |      |      |      |          |      |

先把每一行读取下来

然后分别进行解析

### 解析请求

get请求可以继续读取，并且处理url中的符号

使用snprintf来完成格式化字符串。

对于不存在的文件响应not_found

其他请求则响应badrequest

如果可能，尽量将这些文件放在不同的文件夹中。

### 响应请求

组装好服务器的头部，然后返回客户端一个界面即可。注意过程中要对使用url定位本地文件，使用一个文件夹专门存放html文件，使用拼接来定位文件地址。

| 协议版本 | 空格 | 响应代号 | 空格 | 代号描述 | \r\n |
| -------- | ---- | -------- | ---- | -------- | ---- |
| 字段名   | :    | 值       | \r\n |          |      |
| \r\n     |      |          |      |          |      |
| data     |      |          |      |          |      |

### 开启线程

首先要把请求函数写成线程函数，然后准备线程的相关参数。开启线程，等待其结束。

项目地址(https://github.com/doctordragon666/http-server.git)

公开项目，可以随时参观
