# HTTP Server Based on Linux Platform

An HTTP server developed using pure C language on the Linux platform, without using any frameworks. It can only parse local static files and does not implement the POST method.

## Project Overview

Here is an overview of the files:

```shell
.
├── README.md
├── html_docs # HTML resources, the pages below can be replaced at will
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
├── main # Executable program
├── main.c # Main function
└── run.sh # Compilation, make can be used instead
```

## Core Function Description
Since this project is short and a very simple example program, most of the explanations are placed inside the program.

The core functions are as follows:

```cpp
/// @brief Initiate a request
/// @param pthread_sock Thread handle, client handle
/// @return NULL, thread termination signal
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
        The first line sent by the browser: GET / HTTP/1.1
        */
        strcat(head_buf[METHOD], strtok(read_buf, " "));
        strcat(head_buf[URL], strtok(NULL, " "));
        strcat(head_buf[PROTOOL], strtok(NULL, " "));

        do
        {
            read_len = get_line(client_sock, read_buf, sizeof(read_buf));
            DEBUG(read_buf);
        } while (read_len > 0);// Discard other data, see the output section for details

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
                server_reply.signal = 404; // File not found
                strcpy(server_reply.describe, "NOT FOUND");
            } // File not found, jump to sending request
            else
            {
                if (S_ISDIR(st.st_mode))
                {
                    strcat(path, "index.html");
                }// Empty directory, automatically map to index.html
                server_reply.signal = 200;// Request successful
                strcpy(server_reply.describe, "OK");
            } // Use default path
        }     // GET method
        else
        {
            DEBUG("it is other method\n");
            strcat(path, "./html_docs/unimplement.html");
            server_reply.signal = 501;// Request not implemented
            strcpy(server_reply.describe, "Method Not Implemented");
        } // Other methods do not exist
    }     // Method is correct
    else
    {
        DEBUG("request method error");
        strcat(path, "./html_docs/bad_request.html");
        server_reply.signal = 400;// Request error
        strcpy(server_reply.describe, "BAD REQUEST");
    }
    do_http_response(client_sock, path, server_reply);

    close(client_sock);
    if (pthread_sock)
    {
        free(pthread_sock);
    } // To terminate within the thread function
    return NULL;
}

```

## License Description
This project is based on the MIT license, details can be found in the LICENSE file.

Project Address: (https://github.com/doctordragon666/http-server.git)