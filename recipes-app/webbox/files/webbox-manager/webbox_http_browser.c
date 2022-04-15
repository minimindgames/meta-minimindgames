#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/un.h>
#include <dirent.h> 
#include <sys/wait.h>
#include <vlc/vlc.h>

#include "webbox_http.h"

#define LOG_NAME "BROWSER: "

static int pid = -1;

static int start_browser() {
    int pid = fork();
    switch (pid) {
        case -1:
            perror(LOG_NAME);
            break;
        case 0:
            // child process is run as "weston"
            setuid(1000);
            setgid(1000);
            putenv("HOME=/home/weston");
#ifndef VLC_EXEC
            printf(LOG_NAME "Browser not started on host %s.\n", filename);
            exit(EXIT_SUCCESS);
#endif
            if (execl("/usr/bin/chromium", "chromium", "--start-maximized", NULL) == -1) {
                perror("execl");
            }
            exit(EXIT_SUCCESS);
        default:
            printf(LOG_NAME "Chromium (pid=%d)\n", pid);
            break;
    }
    return pid;
}

static bool stop_browser(int pid) {
    printf(LOG_NAME "Stop pid=%d\n", pid);
    if (kill(pid, SIGTERM) == -1) {
        perror("kill");
        return false;
    }
    int status;
    if (waitpid(pid, &status, WUNTRACED|WCONTINUED) == -1) {
        perror("waitpid");
        return false;
    }
    return true;
}

static bool cmd_start(int sock, const char const *msg) {
    if (pid == -1) {
        pid = start_browser();
    }

    const char *response = (pid != -1) ? "Browser started..." : "Browser failed!";
    if (write(sock, response, strlen(response)) != strlen(response)) {
        perror("send");
    }
    
    return pid != -1;
}

static webbox_http_command commands[] = {
    { .name = "/start", .handle = cmd_start},
};

static bool command(int sock, const char const *path) {
    for (int i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
        if (strncmp(path, commands[i].name, strlen(commands[i].name)) == 0) {
            printf(LOG_NAME "%s\n", path);
            commands[i].handle(sock, path);
            return true;
        }
    }
    return false;
}

webbox_http_command webbox_http_browser = {
    .name = "/browser",
    .handle = command,
};
