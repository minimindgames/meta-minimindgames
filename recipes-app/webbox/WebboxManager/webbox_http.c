#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "webbox_module.h"

#include "webbox_http.h"

// order reversed for canonical path
static webbox_http_command *commands[] = {
    &webbox_http_suspend,
    &webbox_http_img,
    &webbox_http_get,
};

static int sockets[5]; // first socket to listen :80 then some for clients

void http_line(int socket, const char *const text) {
    if (fcntl(sockets[0], F_GETFD) == -1 || errno == EBADF) return;
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

static int http_socket(int port) {
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

    if (listen(sock, 4) == -1) {
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

static bool module_init(int manager_socket) {
    for (int i = 0; i < sizeof(sockets)/sizeof(sockets[0]); i++) 
    {
        sockets[i] = -1;
    }

    sockets[0] = http_socket(80);

    fd_set active_fd_set;

    while (fcntl(sockets[0], F_GETFD) != -1 || errno != EBADF) {
        FD_ZERO(&active_fd_set);
        int fd_set_max = -1;
        for (int i=0 ; i < sizeof(sockets)/sizeof(sockets[0]); i++) {
            int fd = sockets[i];
            if(fd >= 0) {
                FD_SET(fd, &active_fd_set);
            }
            if(fd > fd_set_max) {
                fd_set_max = fd;
            }
        }

        int activity = select(fd_set_max + 1, &active_fd_set, NULL, NULL, NULL);
        if (activity == -1) {
            if (errno != EINTR) {
                perror("select");
            }
            continue;
        }

        if (FD_ISSET(sockets[0], &active_fd_set)) {
            int i;
            for (i = 0; i < sizeof(sockets)/sizeof(sockets[0]); i++) {
                if (sockets[i] == -1) {
                    break;
                }
            }
            if (i == sizeof(sockets)/sizeof(sockets[0])) {
                printf("%s: out of sockets\n", __func__);
                sleep(1);
                continue;
            }

            sockets[i] = http_accept(sockets[0]);
            if (sockets[i] == -1) {
                perror("accept");
                continue;
            }

            FD_SET(sockets[i], &active_fd_set);
        }

        for (int i = 1; i < sizeof(sockets)/sizeof(sockets[0]); i++) {
            int sock = sockets[i];
            if (FD_ISSET(sock, &active_fd_set)) {
                char buffer[1024];
                int n = read(sockets[i], buffer, 1024);
                if (n == -1) {
                    perror("read");
                }
                if (n > 0) {
                    buffer[n] = '\0';
                    http_command(sockets[i], buffer);
                }
                close(sockets[i]);
                sockets[i] = -1;
            }
        }
    }
    return false;
}

static void module_exit(int sig_no) {
    for (int i = 0; i < sizeof(sockets)/sizeof(sockets[0]); i++) {
        if (sockets[i] >= 0) {
            close(sockets[i]);
            sockets[i] = -1;
        }
    }
}

webbox_module webbox_http = {
    .name = __FILE__,
    .init = module_init,
    .exit = module_exit,
};
