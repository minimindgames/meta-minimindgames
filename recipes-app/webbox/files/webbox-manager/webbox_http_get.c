#include <string.h>
#include <stdlib.h>

#include "webbox_http.h"

static bool command_get(int socket, const char const *path) {
    char buf[80];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/html/%sindex.html", path);
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("Get path overflow: %s (%d)\n", buf, len);
        return true;
    }

    http_line(socket, "HTTP/1.1 200 OK\r\n");
    http_line(socket, "Content-Type: text/html; charset=iso-8859-1\r\n");
    http_line(socket, "Connection: close\r\n");
    http_line(socket, "\r\n");

    /*FILE *in_file = fopen(".vlc", "r");
    struct stat sb;
    stat(filename, &sb);
    char *file_contents = malloc(sb.st_size);*/
/*    char *config = playlist_config;
    char folder[80], file[80];
    while (scanf("%s :: %s\n ", folder, file) != EOF) {
        printf("%s :: %s\n", folder, file);
    }*/

    FILE *file = fopen (buf, "rt");
    if (file) {
      fseek (file, 0, SEEK_END);
      long length = ftell (file);
        if (length == -1) {
            perror("ftell");
        } else {
            length++; // for '\0'
            if (fseek (file, 0, SEEK_SET) == 0) {
                char *buffer = (char*)malloc (length);
                if (buffer) {
                    fread (buffer, 1, length, file);
                    buffer[length - 1] = '\0';
                    if (buffer) {
                        if (send(socket, buffer, length, 0) != length) {
                            perror("send");
                        }
                    }
                    free (buffer);
                } else {
                    printf("%s: out of memory\n", __func__);
                }
            }
        }
        fclose (file);
    } else {
        printf("Get path not found: %s\n", buf);
        http_line(socket, "Path not found\n", "\r\n");
    }
    return true;
}

static bool command_image(int socket, const char const *path) {
    char buf[80];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/html/%s", path);
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("Get path overflow: %s (%d)\n", buf, len);
        return true;
    }
    FILE *file = fopen (buf, "rb");
    if (file) {
      fseek (file, 0, SEEK_END);
      long length = ftell (file);
        if (length == -1) {
            perror("ftell");
        } else {
            length++; // for '\0'
            if (fseek (file, 0, SEEK_SET) == 0) {
                char *buffer = (char*)malloc (length);
                if (buffer) {
                    fread (buffer, 1, length, file);
                    buffer[length - 1] = '\0';
                    if (buffer) {
                        http_line(socket, "HTTP/1.1 200 OK\r\n");
                        http_line(socket, "Content-Type: image/png\r\n");
                        http_line(socket, "Connection: close\r\n");
                        http_line(socket, "Content-Length: %d\r\n", length);
                        http_line(socket, "\r\n");
                        if (send(socket, buffer, length, 0) != length) {
                            perror("send");
                        }
                    }
                    free (buffer);
                }
            }
        }
        fclose (file);
    } else {
        printf("Image path not found: %s\n", buf);
        http_line(socket, "Path not found\n", "\r\n");
    }
    return true;
}

static webbox_http_command commands[] = {
    { .name = "img", .handle = command_image},
    { .name = "", .handle = command_get},
};

static bool command(int sock, const char const *path) {
    for (int i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
        if (strncmp(path, commands[i].name, strlen(commands[i].name)) == 0) {
            printf("GET: %s\n", path);
            commands[i].handle(sock, path);
            break;
        }
    }
    return true;
}

webbox_http_command webbox_http_get = {
    .name = "/",
    .handle = command,
};
