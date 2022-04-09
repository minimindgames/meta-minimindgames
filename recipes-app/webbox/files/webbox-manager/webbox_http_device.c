#include "webbox_http.h"

static bool command(int sock, const char const *msg) {
    const char response[] = "Device info";
    if (send(sock, response, sizeof(response), 0) != sizeof(response)) {
        perror("send");
    }
}

webbox_http_command webbox_http_device = {
    .name = "/suspend",
    .handle = command,
};
