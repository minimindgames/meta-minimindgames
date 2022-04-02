#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "webbox_module.h"

bool webbox_module_process(webbox_module *module, int manager_socket) {
    printf("%s: %s\n", __func__, module->name);

    module->pid = -1;
    int pid = fork();
    switch (pid) {
        case -1:
            perror("fork");
            break;
        case 0:
            // child
            if (module->signal_handler) {
                if (signal(SIGTERM, module->signal_handler) == SIG_ERR) {
                    perror("SIGTERM");
                }
            }
            int success = module->process(manager_socket);
            if (!success) {
                // restart
            }
            exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
        default:
            // parent
            module->pid = pid;
            break;
    }
    
    return (pid != -1);
}

void webbox_module_signal(webbox_module *module) {
    printf("%s: %s pid=%d\n", __func__, module->name, module->pid);

    if (module->pid != -1) {
        int ret = kill(module->pid, SIGTERM);
        if (ret == -1) {
            perror("kill");
        }
        int wstatus;
        if (waitpid(module->pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
            perror("waitpid");
        }
        module->pid = -1;
    }
}
