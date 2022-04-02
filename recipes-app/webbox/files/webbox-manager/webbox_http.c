#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>

#include "webbox_module.h"

#include "webbox_http.h"

static int http_socket;

// order reversed for canonical path
static webbox_http_command *commands[] = {
    &webbox_http_power,
    &webbox_http_img,
    &webbox_http_get,
};

void http_line(int socket, const char *const text) {
    if (send(socket, text, strlen(text), 0) != strlen(text)) {
        perror("send");
    }
}

static void http_response(int socket) {
    http_line(socket, "HTTP/1.1 200 OK\r\n");
    http_line(socket, "Content-Type: text/html; charset=iso-8859-1\r\n");
    http_line(socket, "Connection: close\r\n");
    http_line(socket, "\r\n");
}

static int http_listen(int port) {
    printf("%s port=%d\n", __func__, port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("http socket");
    }

    // clean socket when quit
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt");
    }

    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    addr_in.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
    }

    if (listen(sock, 1) == -1) {
        perror("listen");
    }

    return sock;
}

static int http_accept(int http_sock) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_sock = accept(http_sock, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    return client_sock;
}

int http_command(int sock, const char* line) {
    const char *path_start = strchr(line, ' ');
    if (path_start) {
        while(isspace((unsigned char)*path_start)) path_start++;
        //path_start++; // skip '/'
        const char *path_end = strchr(path_start + 1, ' ');
        if (path_end) {
            char path[path_end - path_start];
            strncpy(path, path_start, sizeof(path));
            path[sizeof(path)] = '\0';
            printf("HTTP: %s (%ld)\n", path, strlen(path));
            for (int i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
                webbox_http_command *command = commands[i];
                if (strncmp(path, command->name, strlen(command->name) - 1) == 0) {
                    printf("Command: %s\n", command->name);
                    command->handle(sock, path + strlen(command->name));
                    /*if (!command->handle(sock, path + strlen(command->name))) {
                        http_response(sock);
                    }*/
                    break;
                }
            }
        }
    }
}

static bool process(int manager_socket) {
    http_socket = http_listen(80);

    // client is closed after response so actually polling always just for http_socket
    int nfds = 0;
    struct pollfd poll_fds[1];
    memset(poll_fds, 0 , sizeof(poll_fds));

    poll_fds[0].fd = http_socket;
    poll_fds[0].events = POLLIN;
    nfds++;

    while (fcntl(http_socket, F_GETFD) != -1 || errno != EBADF) {
        int n = poll(poll_fds, nfds, -1);
        printf("http: poll %d\n", n);
        if (n < 0) {
          perror("poll");
          break;
        }

        for (int i = 0; i < n; i++) { // loop active fd count
            for (; i < n; i++) { // find next active fd
                if(poll_fds[i].revents != 0) {
                    break;
                }
            }

            if(poll_fds[i].revents != POLLIN) {
                printf("poll revents %d\n", poll_fds[i].revents);
                break;
            }

            if (poll_fds[i].fd == http_socket) { // new client
                printf("http: new client %d\n", i);
                /*
                    int j;
                    for (j=0; i<sizeof(poll_fds)/sizeof(poll_fds[0]); j++) {
                    if (poll_fds[j].fd == -1) {
                        break;
                    }
                }
                if (j == sizeof(sockets)/sizeof(sockets[0])) {
                    printf("%s: out of sockets\n", __func__);
                    sleep(1);
                    continue;
                }*/

                int client_fd = http_accept(http_socket);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }

                /*
                poll_fds[j].fd = client_fd;
                poll_fds[j].events = POLLIN;
                nfds++;
                */

                char buf[1024];
                int len = read(client_fd, buf, sizeof(buf)-1);
                if (len < 0) {
                    perror("read");
                    break;
                }
                if (len == 0) { // unexpected close
                    continue;
                }

                buf[len] = '\0';
                http_command(client_fd, buf);

                close(client_fd);
                /*
                poll_fds[j].fd = -1;
                nfds--;
                */
            }
        }
    }
    return false;
}

static void signal_handler(int sig_no) {
    close(http_socket);
}

webbox_module webbox_http = {
    .name = __FILE__,
    .process = process,
    .signal_handler = signal_handler,
};
