#include "struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_BUFF 4096

char* timestamp() {
    time_t now;
    time(&now);
    char* timestamp = ctime(&now);

    return timestamp;
}

void parse_http_request(char *request_string, http_request_t *request) {
    char *line;
    char *saveptr;
    int i;

    // Parse request line
    line = strtok_r(request_string, "\r\n", &saveptr);
    strcpy(request->method, strtok(line, " "));
    strcpy(request->path, strtok(NULL, " "));
    
    // Parse headers
    request->header_count = 0;
    while ((line = strtok_r(NULL, "\r\n", &saveptr))) {
        if (strlen(line) == 0) {
            break;
        }
        if (request->header_count < MAX_HEADER_COUNT) {
            strncpy(request->headers[request->header_count], line, MAX_HEADER_SIZE - 1);
            request->header_count++;
        }
    }
}

void handler_function(int connectfd, http_request_t *request) {
    // Get current timestamp
    char* time = timestamp();

    // open requested file
        char filename[512];
        sprintf(filename, "./www%s", request->path);
        int fp = open(filename, O_RDONLY);

    if (strcmp(request->method, "HEAD") == 0) {
        // does that file exist? nah? 404 that B
        if (fp < 0 ) {
            char response[] = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
            if (send(connectfd, response, strlen(response), 0) < 0) {
            perror("Error sending response");
            }
        }

        // get file size
        struct stat filestat;
        fstat(fp, &filestat);
        
        // make header response
        char headers[MAX_HEADER_SIZE];
        sprintf(headers, "HTTP/1.1 200 OK\r\n"
                            "Server: cihttpd\r\n"
                            "Content-Length: %ld\r\n"
                            "Last-Modified: %s\r\n"
                            , filestat.st_size, time); 
        if (send(connectfd, headers, strlen(headers), 0) < 0) {
            perror("Error sending response");
        }
    } else if (strcmp(request->method, "GET") == 0) {
        // does that file exist? nah? 404 that B
        if (fp < 0 ) {
            close(fp);
            int not_foundfp = open("./www/404.html", O_RDONLY);

            // get file size
            struct stat filestat;
            fstat(not_foundfp, &filestat);

            // make response
            char body[MAX_BODY_SIZE];
            read(not_foundfp, body, MAX_BODY_SIZE - 1);
            char response[MAX_BUFF];
            sprintf(response, "HTTP/1.1 200 OK\r\n"
                                "Server: cihttpd\r\n"
                                "Content-Length: %ld\r\n"
                                "Last-Modified: %s\r\n"
                                "\r\n"
                                "%s"
                                , filestat.st_size, time, body);
            if (send(connectfd, response, strlen(response), 0) < 0) {
                perror("Error sending response");
            }
        }

        // get file size
        struct stat filestat;
        fstat(fp, &filestat);

        // make response
        char body[MAX_BODY_SIZE];
        read(fp, body, MAX_BODY_SIZE - 1);
        char response[MAX_BUFF];
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                            "Server: cihttpd\r\n"
                            "Content-Length: %ld\r\n"
                            "Last-Modified: %s\r\n"
                            "\r\n"
                            "%s"
                            , filestat.st_size, time, body);
        if (send(connectfd, response, strlen(response), 0) < 0) {
            perror("Error sending response");
        }
        
    } 
    else {
        // return 501 Not Implemented
        char response[] = "HTTP/1.1 501 Not Implemented\r\nConnection: close\r\n\r\n";
        if (send(connectfd, response, strlen(response), 0) < 0) 
            perror("Error sending response");
    }
}

int main(void) {
    pid_t pid;
    int listenfd, connectfd, n, i;
    socklen_t clilen;
    struct sockaddr_in cliaddr, srvaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr.sin_port = htons(80);

    if (bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) < 0) {
        perror("SERVER BIND ERROR");
        exit(EXIT_FAILURE);
    }

    listen(listenfd, 5);

    for (;;) {
        clilen = sizeof(cliaddr);
        connectfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

        switch(fork()) {
            case -1:
                printf("Error creating child, giving up on client\n");
                close(connectfd);
                break;
            case 0:
                close(listenfd);

                char buff[MAX_BUFF];
                n = read(connectfd, buff, MAX_BUFF);
                buff[n] = 0;

                http_request_t request;

                parse_http_request(buff, &request);

                // printf("Method: %s\n", request.method);
                // printf("Path: %s\n", request.path);
                // printf("Headers:\n");
                // for (i = 0; i < request.header_count; i++) {
                //     printf("%s\n", request.headers[i]);
                // }
                // printf("Body: %s\n", request.body);
                
                handler_function(connectfd, &request);

                exit(0);
            
            default:
                close(connectfd);
        }
        
    }

    return EXIT_SUCCESS;
}