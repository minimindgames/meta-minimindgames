#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "webbox_module.h"

static bool process(int manager_socket) {
    while (fcntl(STDIN_FILENO, F_GETFD) != -1 || errno != EBADF) {
        char buf[80];
        int rc = read(STDIN_FILENO, buf, sizeof(buf));
        int wc = write(manager_socket, buf, rc);
        if (rc != wc) {
            perror("write");
        }
    }
    return false;
}

static void signal_handler(int sig_no) {
    close(STDIN_FILENO);
}

webbox_module webbox_console = {
    .name = __FILE__,
    .process = process,
    .signal_handler = signal_handler,
};
