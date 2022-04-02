#include <stdbool.h>

typedef struct {
    const char const *name;
    bool (*process)(int manager_socket);
    void (*signal_handler)(int sig_no);
    int pid;
} webbox_module;

bool webbox_module_process(webbox_module *module, int manager_socket);
void webbox_module_signal(webbox_module *module);

extern webbox_module webbox_console;
extern webbox_module webbox_browser;
extern webbox_module webbox_http;
extern webbox_module webbox_idle;
