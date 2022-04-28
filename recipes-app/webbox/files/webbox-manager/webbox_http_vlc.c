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

static const char vlc_socket_name[] = "/run/user/1000/vlc.sock";

#define LOG_NAME "VLC: "

#define PLAYLIST_PATH WEBBOX_LIB_PATH "/playlists"
#define LATEST_FILENAME "/vlc_latest.cfg"
#define PLAYLIST_SKIP "/home/weston/.webbox/playlists.skip"

static int vlc_pid = -1;

typedef struct playlist {
    char *folder;
    struct playlist *more;
} playlist;

//static playlist *playlists;

static void playlist_clear(playlist *p) {
    while (p) {
        playlist *next = p->more;
        free(p->folder);
        free(p);
        p = next;
    }
}

// use "" for empty folder/name
static char *get_path(const char* const folder, const char* const name) {
    char *filename = NULL;

    filename = malloc(strlen(PLAYLIST_PATH) + strlen(folder) + 1 + strlen(name) + 1); // +1 for '/' and '\0'
    if (filename) {
        (void)sprintf(filename, "%s%s%s", PLAYLIST_PATH, folder, name);
    }

    return filename;
}

static char *read_file(const char* const filename) {
    FILE *file = NULL;
    char *contents = NULL;

    file = fopen(filename, "rt");
    if (!file) {
        goto out;
    }
    
    if (fseek(file, 0, SEEK_END) != 0) {
        goto out;
    }
    long length = ftell(file);
    if (length == -1) {
        goto out;
    }
    if (fseek (file, 0, SEEK_SET) != 0) {
        goto out;
    }

    contents = malloc(length+1);
    if (!contents) {
        goto out;
    }
    if (fread(contents, 1, length, file) != length) {
        goto out;
    }
    contents[length] = '\0';
    printf(LOG_NAME "Read %s=%s\n", filename, contents);

out:
    if (!contents) {
        printf(LOG_NAME "No file %s\n", filename);
    }
    if (file) {
        fclose(file);
    }

    return contents;
}

static void write_file(const char* const filename, const char* const contents) {
    FILE *file = NULL;

    file = fopen(filename, "wt");
    if (file) {
        int length = strlen(contents);
        if (fwrite(contents, 1, length, file) == length) {
            printf(LOG_NAME "Write %s: %s\n", filename, contents);
        } else {
            printf(LOG_NAME "Write file failed %s: %s\n", filename, contents);
        }
        fclose(file);
    }
}

static int dir_filter(const struct dirent *entry) {
   return (entry->d_type == DT_DIR && entry->d_name[0] != '.');
}

static int file_filter(const struct dirent *entry) {
    if (entry->d_type == DT_DIR) { // to recurse
        return 1;
    }
    if (entry->d_type != DT_REG) {
        return 0;
    }
    char *types[] =  { "wav", "aac", "mp3", "wma", "flac", "m4a" };
    for (int i=0; i<sizeof(types)/sizeof(types[0]); i++) {
        char *type = types[i];
        int len = strlen(entry->d_name);
        int n = strlen(type);
        if (len > n) {
            if (strcmp(entry->d_name + len - n, type) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

playlist *get_dir(playlist *playlists, const char* const folder, bool recurse) {
    char *filename = NULL;

    filename = get_path(folder, "");
    if (!filename) {
        return NULL;
    }

    struct dirent **namelist;
    printf("get_dir: %s\n", filename);
    int n = scandir(filename, &namelist, (recurse) ? file_filter : dir_filter, alphasort);
    if (n < 0) {
        perror("scandir");
        goto out;
    }

    playlist *last = playlists;
    while (last && last->more) {
        last = last->more;
    }

    for (int i=0; i<n; i++) {
        if (recurse && namelist[i]->d_type == DT_DIR) {
            if (namelist[i]->d_name[0] != '.') {
                char *path = malloc(strlen(folder) + 1 + strlen(namelist[i]->d_name) + 1);
                (void)sprintf(path, "%s/%s", folder, namelist[i]->d_name);
                playlists = get_dir(playlists, path, recurse);
                free(path);
            }
            continue;
        }

        playlist *p = (playlist *)malloc(sizeof(playlist));
        p->folder = malloc(strlen(folder) + 1 + strlen(namelist[i]->d_name) + 1);
        (void)sprintf(p->folder, "%s/%s", folder, namelist[i]->d_name);

        free(namelist[i]);

        if (last) {
            last->more = p;
        } else {
            playlists = p;
            last = playlists;
        }
        last = p;
    }
    if (last) {
        last->more = NULL;
    }
    free(namelist);

out:
    free(filename);

    return playlists;
}

playlist *get_latest(playlist *list, const char* const folder) {
    if (!list) {
        return NULL;
    }
    
    char *filename = get_path(folder, LATEST_FILENAME);
    char *latest = read_file(filename);
    free(filename);
    if (!latest) {
        return list;
    }

    playlist *p = list;
    while (p) {
        if (strcmp(latest, p->folder) == 0) {
            break;
        }
        p = p->more;
    }
    free(latest);

    if (!p) {
        return list;
    }

    return p;
}

static int start_vlc(const char const *folder) {
    int pid = fork();
    switch (pid) {
        case -1:
            perror("VLC");
            break;
        case 0:
            // child process is run as "weston" for audio resources
            setuid(1000);
            setgid(1000);
            putenv("HOME=/home/weston");

            libvlc_instance_t *inst = libvlc_new (0, NULL);
            if (!inst) {
                exit(EXIT_FAILURE);
            }
            libvlc_media_player_t *mp;
            playlist *files = get_dir(NULL, folder, true);

            /*printf("Playlist files:\n");
            playlist *debug = files;
            while (debug) {
                printf("%s\n", debug->folder);
                debug = debug->more;
            }*/

            playlist *p = get_latest(files, folder);
            char *fskiplist = read_file(PLAYLIST_SKIP);
            if (p->more) { // do not continue always from the first one
                p = p->more;
            } else {
                p = files;
            }
            while (p) { // loop playlist
                char *filename = get_path("", p->folder);
                if (access(filename, F_OK ) != 0) {
                    printf("Check file read permission %s\n", filename);
                    free(filename);
                    p = p->more;
                    continue;
                }

                if (fskiplist) {
                    char *line = strtok(fskiplist, "\n");
                    bool skip = false;
                    while(line) {
                        if (strcmp(line, filename) == 0) {
                            skip = true;
                            break;
                        }
                        line = strtok(NULL, "\n");
                    }
                    if (skip) {
                        printf("Skip %s\n", filename);
                        p = p->more;
                        continue;
                    }
                }

                char *latest = get_path(folder, LATEST_FILENAME);
                write_file(latest, p->folder);
                free(latest);

#ifndef VLC_EXEC
                printf(LOG_NAME "No audio play on host %s.\n", filename);
                free(filename);
                playlist_clear(files);
                free(fskiplist);
                exit(EXIT_SUCCESS);
#endif

                libvlc_media_t *m;
                //m = libvlc_media_new_location (inst, url);
                m = libvlc_media_new_path (inst, filename);
                free(filename);
                if (!m) {
                    printf(LOG_NAME "Media failure\n");
                    exit(EXIT_FAILURE);
                }

                mp = libvlc_media_player_new_from_media (m);
                if (mp) {
                   if (libvlc_media_player_play (mp) == -1) {
                        printf("VLC play failed!");
                        exit(EXIT_FAILURE);
                   }
                }

                libvlc_media_parse (m); // need to call before libvlc_media_get_duration
                int seconds = (int)(ceil(libvlc_media_get_duration(m)/1000));
                printf(LOG_NAME "Media time %d\n", seconds);

                if (sleep(seconds) != 0) {
                    printf(LOG_NAME "Play interrupted.");
                    libvlc_media_release (m);
                    exit(EXIT_FAILURE);
                }

                libvlc_media_release (m);

                if (p->more) { // loop forever
                    p = p->more;
                } else {
                    p = files;
                }
            }
    
            libvlc_media_player_stop (mp);
            libvlc_media_player_release (mp);
            libvlc_release (inst);
            free(fskiplist);
            exit(EXIT_SUCCESS);
        default:
            printf(LOG_NAME "Playlist %s (pid=%d)\n", folder, pid);
            break;
    }
    return pid;
}

static bool stop_vlc(int pid) {
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


static bool cmd_load(int sock, const char const *msg) {
    playlist *playlists = get_dir(NULL, "", false);
    if (playlists) {
        playlist *p = playlists;
        char *response = "Loaded playlists\n";
        if (write(sock, response , strlen(response )) != strlen(response)) {
            perror("send");
        }
        while (p) {
            if (write(sock, p->folder+1, strlen(p->folder)-1) != strlen(p->folder)-1) {
                perror("send");
            }
            if (write(sock, "\n", strlen("\n")) != strlen("\n")) { // separated by newlines
                perror("send");
            }
            p = p->more;
        }
    } else {
        char *response = "No playlists.\n";
        if (write(sock, response, strlen(response)) != strlen(response)) {
            perror("send");
        }
    }
    playlist_clear(playlists);
    return true;
}

static bool cmd_play(int sock, const char const *msg) {
    const char cmd[] = "play";

    const char *response = NULL;
    char *folder = strdup(msg + sizeof(cmd));
    if (folder[0] != 0) {
        char *filename = get_path("", LATEST_FILENAME);
        FILE *file = fopen (filename, "wt");
        free(filename);
        if (file) {
            fprintf(file, "%s", folder);
            fclose (file);
        }
        response = folder;
    } else {
	if (vlc_pid != -1) {
            printf(LOG_NAME "Already playing\n");
            response = folder;
            folder = NULL;
	} else {
            playlist *playlists = get_dir(NULL, "", false);
            playlist *p = get_latest(playlists, "");
            if (p) {
                folder = strdup(p->folder);
                response = folder;
            } else {
                printf("No playlists. Link %s to your music folders where 'weston' has access rights.\n", PLAYLIST_PATH);
                folder = NULL;
            }
            playlist_clear(playlists);
        }
    }

    if (folder) {
        if (vlc_pid != -1) {
            stop_vlc(vlc_pid);
        }
        vlc_pid = start_vlc(folder);
    }

    if (!response) {
        response = "Failed!";
    }
    if (write(sock, response, strlen(response)) != strlen(response)) {
        perror("send");
    }
    free(folder);

    return vlc_pid != -1;
}

static bool cmd_stop(int sock, const char const *msg) {
    bool success = true;
    if (vlc_pid != -1) {
        if (!stop_vlc(vlc_pid)) {
            success = false;
            // should restart all
        }
        vlc_pid = -1;
    }
    const char ok[] = "Stopped.";
    const char nok[] = "Stop failed!";
    const char *response = success ? ok : nok;
    if (send(sock, response, strlen(response), 0) != strlen(response)) {
        perror("send");
    }
    return true;
}

static bool cmd_skip(int sock, const char const *msg) {
    const char *response = NULL;

    if (vlc_pid == -1) {
        response = "Nothing playing.";
    } else {
        playlist *playlists = get_dir(NULL, "", false);
        playlist *p = get_latest(playlists, "");
        if (p) {
            char *filename = get_path(p->folder, LATEST_FILENAME);
            if (filename) {
                char *latest = read_file(filename);
                free(filename);
                if (latest) {
                    char *fskip = get_path("", latest);
                    if (fskip) {
                        if (access(fskip , F_OK ) == 0) {
                            FILE *file = fopen(PLAYLIST_SKIP, "at");
                            if (file) {
                                int length = strlen(fskip);
                                if (fwrite(fskip, 1, length, file) == length) {
                                    if (fwrite("\n", 1, 1, file) == 1) {
                                        printf(LOG_NAME "Skip %s: %s\n", PLAYLIST_SKIP, fskip);
                                        response = "Skipped";
                                    } else {
                                        printf(LOG_NAME "Skip failed %s\n", PLAYLIST_SKIP);
                                    }
                                } else {
                                    printf(LOG_NAME "Skip failed %s\n", PLAYLIST_SKIP);
                                }
                                fclose(file);
                                if (response) {
                                    if (vlc_pid != -1) {
                                        stop_vlc(vlc_pid);
                                    }
                                    vlc_pid = start_vlc(p->folder);
                                }
                            }
                        } else {
                            perror("access");
                        }
                        free(fskip);
                    }
                    free(latest);
                }
            }
        }
        playlist_clear(playlists);
    }

    if (!response) {
        response = "Skip failed, check file write permissions.";
    }
    printf(LOG_NAME "%s\n", response);
    if (send(sock, response, strlen(response), 0) != strlen(response)) {
        perror("send");
    }
    return true;
}

static webbox_http_command commands[] = {
    { .name = "/load", .handle = cmd_load},
    { .name = "/play", .handle = cmd_play},
    { .name = "/stop", .handle = cmd_stop},
    { .name = "/skip", .handle = cmd_skip},
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

static void command_exit(void) {
    printf("command_exit\n");
}

webbox_http_command webbox_http_vlc = {
    .name = "/VLC",
    .handle = command,
    .exit = command_exit,
};
