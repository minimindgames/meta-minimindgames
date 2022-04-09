#include <stdlib.h>
#include <string.h>

#include "webbox_http.h"

static bool suspend(int sock, const char const *msg) {
    if (system("/usr/bin/systemctl suspend") != 0) {
        const char response_failed[] = "Suspend failed";
        if (send(sock, response_failed, sizeof(response_failed), 0) != sizeof(response_failed)) {
            perror("send");
        }
        return true;
    }
    const char response[] = "Suspending...";
    if (send(sock, response, sizeof(response), 0) != sizeof(response)) {
        perror("send");
    }
    return true;
}

static webbox_http_command commands[] = {
    { .name = "/suspend", .handle = suspend},
};

static bool command(int sock, const char const *path) {
    for (int i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
        printf("%s %s\n", commands[i].name, path);
        if (strncmp(path, commands[i].name, strlen(commands[i].name)) == 0) {
            commands[i].handle(sock, path);
        }
    }
    return true;
}

webbox_http_command webbox_http_power = {
    .name = "/power",
    .handle = command,
};
