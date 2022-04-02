#include <stdlib.h>

#include "webbox_http.h"

static void command(int socket, const char const *path) {
    char buf[80];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/html/%sindex.html", path);
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("Path overflow: %s (%d)\n", buf, len);
        return;
    }

    http_line(socket, "HTTP/1.1 200 OK\r\n");
    http_line(socket, "Content-Type: text/html; charset=iso-8859-1\r\n");
    http_line(socket, "Connection: close\r\n");
    http_line(socket, "\r\n");

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
        printf("Path not found: %s\n", buf);
    }
}

webbox_http_command webbox_http_get = {
    .name = "/",
    .handle = command,
};
