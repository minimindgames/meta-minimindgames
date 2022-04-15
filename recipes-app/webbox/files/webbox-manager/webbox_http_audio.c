#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "webbox_http.h"

static bool cmd_volume(int sock, const char const *msg) {
    char buf[80];
    char *vol;

    msg++; // skip '/'
    if (strcmp(msg, "down") == 0) {
        vol = "-";
    } else {
        vol = "+";
    }

//    snprintf(buf, sizeof(buf), "/usr/bin/pactl set-sink-volume @DEFAULT_SINK@ %s", vol);
//    snprintf(buf, sizeof(buf), "/usr/bin/amixer -c0 set PCM 5%%%s", vol);
    snprintf(buf, sizeof(buf), "/usr/bin/amixer -c0 set Master 5%%%s", vol);
#if VLC_EXEC
    if (system(buf) != 0) {
        const char response_failed[] = "Volume failed.";
        if (send(sock, response_failed, sizeof(response_failed), 0) != sizeof(response_failed)) {
            perror("send");
        }
        return true;
    }
#else
    printf("Skip on host: %s\n", buf);
#endif
    /*int pid = fork();
    switch (pid) {
        case -1:
            perror("vlc process");
            break;
        case 0: {
                if (execl("/usr/bin/pactl", "pactl", "set-sink-volume", "@DEFAULT_SINK@", vol, NULL) == -1) {
                    perror("execl");
                }
                exit();
            } break
        default: 
            int status;
            if (waitpid(pid, &status, WUNTRACED|WCONTINUED) == -1) {
                perror("waitpid");
                return;
            }
            const char response_failed[] = "Volume failed";
            if (send(sock, response_failed, sizeof(response_failed), 0) != sizeof(response_failed)) {
                perror("send");
            }
            break;
    }
*/
    const char response[] = "Volume tuned.";
    if (send(sock, response, sizeof(response), 0) != sizeof(response)) {
        perror("send");
    }
    return true;
}

static webbox_http_command commands[] = {
    { .name = "/volume", .handle = cmd_volume},
};

static bool command(int sock, const char const *path) {
    for (int i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
        printf("audio: %s %s\n", commands[i].name, path);
        if (strncmp(path, commands[i].name, strlen(commands[i].name)) == 0) {
            commands[i].handle(sock, path + strlen(commands[i].name));
        }
    }
    return true;
}

webbox_http_command webbox_http_audio = {
    .name = "/audio",
    .handle = command,
};
