#include <stdio.h>
#include <netinet/ip.h>

typedef struct {
    const char const *name;
    void (*handle)(int sock, const char const *params);
} webbox_http_command;

extern void http_line();

extern webbox_http_command webbox_http_get;
extern webbox_http_command webbox_http_img;
extern webbox_http_command webbox_http_suspend;
