#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "webbox_module.h"

static bool module_init(int manager_socket) {
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

static void module_exit(int sig_no) {
    close(STDIN_FILENO);
}

webbox_module webbox_console = {
    .name = __FILE__,
    .init = module_init,
    .exit = module_exit,
};
