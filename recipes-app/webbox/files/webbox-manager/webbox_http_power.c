#include <stdlib.h>

#include "webbox_http.h"

static void command(int sock, const char const *msg) {
    if (system("/usr/bin/systemctl suspend") != 0) {
        const char response_failed[] = "Suspend failed";
        if (send(sock, response_failed, sizeof(response_failed), 0) != sizeof(response_failed)) {
            perror("send");
        }
        return;
    }
    const char response[] = "Suspending...";
    if (send(sock, response, sizeof(response), 0) != sizeof(response)) {
        perror("send");
    }
}

webbox_http_command webbox_http_power = {
    .name = "/power/",
    .handle = command,
};
