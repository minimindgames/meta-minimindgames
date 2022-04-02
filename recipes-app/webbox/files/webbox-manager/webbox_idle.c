#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

//#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <errno.h>

#include "webbox_module.h"

static int sockets[2]; // js-dev, manager_socket

static bool process(int manager_socket) {
    for (int i = 0; i < sizeof(sockets)/sizeof(sockets[0]); i++) 
    {
        sockets[i] = -1;
    }

    const char* const dev = "/dev/tty0";
    //const char* const dev = "/dev/input/js0";
    //struct input_event ev;

    int fd = open(dev, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open");
        return false;
    }

    fd_set active_fd_set;

    sockets[0] = fd;
    //sockets[1] = manager_socket;

    while (fcntl(fd, F_GETFD) != -1 || errno != EBADF) {
        FD_ZERO(&active_fd_set);
        int fd_set_max = -1;
        for (int i=0 ; i < sizeof(sockets)/sizeof(sockets[0]); i++) {
            int fd = sockets[i];
            if(fd >= 0) {
                FD_SET(fd, &active_fd_set);
            }
            if(fd > fd_set_max) {
                fd_set_max = fd;
            }
        }

        int activity = select(fd_set_max + 1, &active_fd_set, NULL, NULL, NULL);
        if (activity == -1) {
            if (errno != EINTR) {
                perror("select");
            }
            continue;
        }

        for (int i = 0; i < sizeof(sockets)/sizeof(sockets[0]); i++) {
            int sock = sockets[i];
            if (FD_ISSET(sock, &active_fd_set)) {
                char buffer[1024];
                int n = read(sockets[i], buffer, 1024);
                printf("idle read %d\n", n);
                if (n == -1) {
                    perror("read");
                }
                if (n > 0) {
                    buffer[n] = '\0';
                    printf("idle: %s\n", buffer);
                    //http_command(manager_socket, buffer);
                    //http_response(sockets[i]);
                }
                //close(sockets[i]);
                //sockets[i] = -1;
            }
        }
    }

    /*if (execl("/usr/bin/systemctl", "systemctl", "suspend", NULL) == -1) {
        perror("suspend");
    }*/
/*    while (fcntl(STDIN_FILENO, F_GETFD) != -1 || errno != EBADF) {
        char buf[80];
        int rc = read(STDIN_FILENO, buf, sizeof(buf));
        int wc = write(manager_socket, buf, rc);
        if (rc != wc) {
            perror("write");
        }
    }*/
    return false;
}

static void signal_handler(int sig_no) {
    close(sockets[0]);
}

webbox_module webbox_idle = {
    .name = __FILE__,
    .process = process,
    .signal_handler = signal_handler,
};


#if 0
#include <stdio.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <linux/joystick.h>
#include <stdlib.h> // for system
#include <errno.h>

int main(int argc, char *argv[]) {
    const char *dev;
    int fd;
//    struct js_event ev;
    struct input_event ev;

    if (argc > 1) {
        dev = argv[1];
    } else {
        dev = "/dev/input/js0";
    }

    fd = open(dev, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        printf("No device %s\n", dev);
        return -1;
    }

fd_set rfds;
FD_ZERO(&rfds);
FD_SET(fd, &rfds);

    while (1) {
int res = select(fd + 1, &rfds, NULL, NULL, NULL);
    if ( -1 == res && EINTR == errno ) {
printf("cont\n");
      continue;
    }
    if ( -1 == res ) {
      perror("select() failed");
      // fprintf(stderr, "failed to select, fd is %d\n", fd);
      return -1;
    }
if ( FD_ISSET(fd, &rfds) ) {
    printf("got event\n");
} else {
    continue;
}
        ssize_t n;
        n = read(fd, &ev, sizeof(ev));
if (n != -1)
	printf("size %d\n", (int)n);
        if (n != sizeof(ev)) {
            continue;
        }
/*        if (ev.type == JS_EVENT_BUTTON) {
            switch (ev.number) {
                default:
                    printf("Button %u %s\n", ev.number, ev.value ? "pressed" : "released");
                    break;
            }
        }*/
        if (ev.type == EV_KEY) {
            switch (ev.code) {
                default:
                    printf("Button %u %s\n", ev.code, ev.value ? "pressed" : "released");
if (ev.code == 1 && ev.value == 0) {
    pid_t p = fork();
    if (p == -1) {
        printf("Error");
        continue;
    }
    if (p == 0) {
//        const char *cmd[] = {""};
//        execv("/usr/bin/idle.sh");
        return system("/usr/bin/idle.sh");
    }
}
                    break;
            }
        }
    }

    // actually never here
    close(fd);
    return 0;
}
#endif