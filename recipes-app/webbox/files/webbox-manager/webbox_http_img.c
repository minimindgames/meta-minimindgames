#include <stdlib.h>

#include "webbox_http.h"

static void command(int socket, const char const *path) {
    char buf[80];
    int len = snprintf(buf, sizeof(buf), "/usr/lib/webbox-manager/html/img%s", path);
    if (len > sizeof(buf)) {
        return;
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
    }
}

webbox_http_command webbox_http_img = {
    .name = "/img",
    .handle = command,
};
