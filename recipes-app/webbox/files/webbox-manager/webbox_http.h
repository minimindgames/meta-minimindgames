#include <stdio.h>
#include <stdbool.h>
#include <netinet/ip.h>

/*#ifdef CMAKE_CROSSCOMPILING // https://cmake.org/cmake/help/latest/variable/CMAKE_CROSSCOMPILING.html#variable:CMAKE_CROSSCOMPILING
f
#define WEBBOX_LIB_PATH "/usr/lib/webbox-manager"
#else
#define WEBBOX_LIB_PATH ".." // parent of build/
#endif
*/
typedef struct {
    const char const *name;
    void (*init)(void);
    bool (*handle)(int sock, const char const *params);
    void (*exit)(void);
} webbox_http_command;

extern void http_line();

extern webbox_http_command webbox_http_get;
extern webbox_http_command webbox_http_img;
extern webbox_http_command webbox_http_power;
extern webbox_http_command webbox_http_vlc;
extern webbox_http_command webbox_http_audio;
