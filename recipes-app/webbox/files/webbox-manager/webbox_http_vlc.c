#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/un.h>
#include <dirent.h> 
#if VLC
#include <sys/wait.h>
#include <vlc/vlc.h>
#endif

#include "webbox_http.h"

static const char vlc_socket_name[] = "/run/user/1000/vlc.sock";

static int vlc_pid = -1;

#if VLC

typedef struct playlist {
    char *folder;
    struct playlist *more;
} playlist;

playlist *get_dir(char *name, bool rotate) {
    playlist *playlists = NULL;

    char latest[1024];
    latest[0] = '\0';
    char buf[1024];
    int len = snprintf(buf, sizeof(buf), "%s/.vlc_latest", name);
    if (len >= sizeof(buf)) {
        buf[sizeof(buf)-1] = '\0';
        printf("VLC path: %s (%d)\n", buf, len);
    }
    FILE *file = fopen (buf, "rt");
    if (file) {
        char config[1024];
        if (fgets(latest, sizeof(latest), file) == NULL) {
            latest[0] = '\0';
        }
        fclose (file);
    }
    printf("VLC recalls %s = %s\n", buf, latest);

    playlist *latest_found = NULL;
    playlist *last = playlists;

    struct dirent **namelist;
    int n = scandir(name, &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        for (int i=0; i<n; i++) {
            if (namelist[i]->d_name[0] == '.') continue;

            playlist *p = (playlist *)malloc(sizeof(playlist));
            p->folder = strdup(namelist[i]->d_name);
            free(namelist[i]);
            p->more = NULL;

            if (last) {
                last->more = p;
            } else {
                playlists = p;
            }

            if (latest && !latest_found) {
                if (strncmp(latest, p->folder, strlen(p->folder)) == 0) {
                    latest_found = (rotate) ? p : last;
                }
            }

            last = p;
        }
        free(namelist);
    }

    if (latest_found && latest_found->more) {
        last->more = playlists; // loop from beginning
        playlists = latest_found->more;
        latest_found->more = NULL;
    }


    /*playlist *p = playlists;
    while (p) {
        printf("playlist %s\n", p->folder);
        p = p->more;
    }*/

    return playlists;
}

static playlist *get_files(const char const *path) {
    char buf[80];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/playlists/%s", path);
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("VLC path: %s (%d)\n", buf, len);
        return NULL;
    }

    playlist *p = get_dir(buf, true);
    if (!p) {
        printf("No playlist.\n");
    }
    return p;
}

static int start_vlc(const char const *folder) {
    int pid = fork();
    switch (pid) {
        case -1:
            perror("vlc process");
            break;
        case 0: {
/*    for (;;)
    {
      printf("Child Process. parent process id is %u\n", getppid());
      sleep(1);
    }*/
            // child
            setuid(1000);
            setgid(1000);
            putenv("HOME=/home/weston");

            libvlc_instance_t *inst = libvlc_new (0, NULL);
            libvlc_media_player_t *mp;
            
            playlist *p = get_files(folder);
//setvbuf(stdout, 0, _IONBF, 0);
while (p) {
    char buf[1024];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/playlists/%s/%s", folder, p->folder);
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("VLC path: %s (%d)\n", buf, len);
	break;
    }

     libvlc_media_t *m;
    char latest_config[1024];
//printf(WEBBOX_LIB_PATH "/playlists/%s/.vlc_latest\n", folder);
    int latest_len = snprintf(latest_config, sizeof(latest_config),  WEBBOX_LIB_PATH "/playlists/%s/.vlc_latest", folder);
    if (latest_len >= sizeof(latest_config)) {
        latest_config[sizeof(latest_config)-1] = '\0';
        printf("VLC path: %s (%d)\n", latest_config, latest_len);
	break;
    }
    FILE *file = fopen (latest_config, "wt");
    if (file) {
        fprintf(file, "%s\n", p->folder);
        fclose (file);
    }        

#if !CMAKE_CROSSCOMPILING
//    break;
#endif
    //m = libvlc_media_new_location (inst, url);
     m = libvlc_media_new_path (inst, buf);
     if (!m) {
        printf("VLC path failed %s\n", buf);
        break;
     }

     mp = libvlc_media_player_new_from_media (m);

/*    vlc_sem_t sem;
    vlc_sem_init(&sem, 0);

    libvlc_event_manager_t *p_em = libvlc_media_player_event_manager(mp);
    libvlc_event_attach(p_em, libvlc_MediaPlayerPlaying, finished_event, &sem);
    libvlc_event_attach(p_em, libvlc_MediaPlayerEndReached, finished_event, &sem);
    libvlc_event_attach(p_em, libvlc_MediaPlayerEncounteredError, finished_event, &sem);
*/
     if (mp) {
        if (libvlc_media_player_play (mp) == -1) {
		printf("Can't play");
		break;
	}
     } else {
        printf("VLC failed %s\n", buf);
        break;
     }


libvlc_media_parse (m);
int s_length = (int)(ceil(libvlc_media_get_duration(m)/1000));
printf("playing time %d\n", s_length);

int t = sleep(s_length);

//printf("woke from sleep\n");
/*
    vlc_sem_wait (&sem);
*/

/*

    libvlc_event_detach(p_em, libvlc_MediaPlayerPlaying, finished_event, &sem);
    libvlc_event_detach(p_em, libvlc_MediaPlayerEndReached, finished_event, &sem);
    libvlc_event_detach(p_em, libvlc_MediaPlayerEncounteredError, finished_event, &sem);

    libvlc_media_player_stop(p_mp);

    vlc_sem_destroy (&sem);
*/
/*
    libvlc_state_t state;
    do {
        state = libvlc_media_player_get_state (mp);
    } while(state != libvlc_Playing &&
            state != libvlc_Error &&
            state != libvlc_Ended );

    state = libvlc_media_player_get_state (mp);
    assert(state == libvlc_Playing || state == libvlc_Ended);
*/
//		libvlc_event_attach (playerEventManager, libvlc_MediaPlayerEndReached,
//				ListEventCallback, this);
//sleep(10);
     libvlc_media_release (m);
 

if (t != 0) {
	break;
}
     p = p->more;
}
    
     /* Stop playing */
     libvlc_media_player_stop (mp);
 
     /* Free the media_player */
     libvlc_media_player_release (mp);
 
     libvlc_release (inst);

            exit(EXIT_SUCCESS);
        }
        default: {
            printf("Play folder %s\n", folder);
        }   break;
    }
    return pid;
}

static void stop_vlc(int pid) {
    if (kill(pid, SIGTERM) == -1) {
        perror("kill");
        return;
    }
    int status;
    if (waitpid(pid, &status, WUNTRACED|WCONTINUED) == -1) {
        perror("waitpid");
        return;
    }
    printf("VLC stopped\n");
}

#endif // VLC

static bool vlc_command(const char const *cmd, const char const *playlist) {
printf("vlc_command %s %s\n", cmd, playlist);
#if VLC
    if (strcmp(cmd, "/stop") == 0) {
	    if (vlc_pid != -1) {
        	stop_vlc(vlc_pid);
	        vlc_pid = -1;
	    }

        return true;
    }

// stop even same playlist as it's easy way to skip to next song
if (vlc_pid != -1) {
       	stop_vlc(vlc_pid);
        vlc_pid = -1;
}

    vlc_pid = start_vlc(playlist);
printf("started vlc %d\n", vlc_pid);
/*
    int fd;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      perror("VLC socket");
      return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0x0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, vlc_socket_name, sizeof(vlc_socket_name));
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        if (write(fd, cmd, sizeof(cmd)) != sizeof(cmd)) {
            perror("write");
            close(fd);
            return false;
        }
    } else {
        perror("VLC connect");
        close(fd);
        return false;
    }
    close(fd);*/
#endif
    return vlc_pid != -1;
}

static playlist *playlists;

static char *playlist_config;

static void playlist_config_init() {    
    FILE *file = fopen (".vlc", "rt");
    if (file) {
      fseek (file, 0, SEEK_END);
      long length = ftell (file);
        if (length == -1) {
            perror("ftell");
        } else {
            length++; // for '\0'
            if (fseek (file, 0, SEEK_SET) == 0) {
                playlist_config = (char*)malloc (length);
            }
        }
        fclose (file);
    }
}

static void playlist_config_update(const char const *folder, const char const *file) {
    char *new_config = malloc (80);
    int len = snprintf(new_config, 80, "%s :: %s\n", folder, file);
    if (len > 0) {
        free(playlist_config);
        playlist_config = new_config;
    }

    FILE *f = fopen (".vlc", "w");
    if (f) {
        fwrite(playlist_config, 1, strlen(playlist_config), f);
        fclose (f);
    }
}




/*
static bool cmd_play(int sock, const char const *msg) {
    const char cmd[] = "\nplay\n";
    bool success = vlc_command(cmd);
    
    const char ok[] = "Playing...";
    const char nok[] = "Play failed!";
    const char *response = success ? ok : nok;
    if (send(sock, response, strlen(response), 0) != strlen(response)) {
        perror("send");
    }
    return true;
}
*/

static bool cmd_load(int sock, const char const *msg) {
    char buf[80];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/playlists");
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("VLC path: %s (%d)\n", buf, len);
        return true;
    }
    
    playlists = get_dir(buf, false);
    if (!playlists) {
        char *err = "No playlists. Create link to music.\n";
	    if (send(sock, err, strlen(err), 0) != strlen(err)) {
        	perror("send");
	    }
        return true;
    }

    char playlist_buf[10*1024];
    int n = 0;
    playlist *p = playlists;
    n += snprintf(playlist_buf + n, sizeof(playlist_buf), "Loaded playlists\n");
    while (p) {
        n += snprintf(playlist_buf + n, sizeof(playlist_buf), "%s\n", p->folder);
        p = p->more;
    }
//printf("pl %s\n", playlist_buf);
    if (send(sock, playlist_buf, n, 0) != n) {
        perror("send");
    }
    return true;
}

static bool cmd_play(int sock, const char const *msg) {
    const char cmd[] = "play";

    const char *folder = msg + sizeof(cmd);
    if (folder[0] != 0) {
        folder++; // skip '/'

        // add new playlist
        playlist *p = (playlist *)malloc(sizeof(playlist));
        p->folder = strdup(folder);
        p->more = playlists;
        playlists = p;

        // remove previous playlist if exists
        p = playlists;
        while (p) {
            //printf("%s\n", p->folder);
            if (p->more) {
                if (strcmp(folder, p->more->folder) == 0) {
                    playlist *remove = p->more;
                    p->more = remove->more;
                    free(remove->folder);
                    free(remove);
                }
            }
            p = p->more;
        }

        char latest_config[1024];
        int latest_len = snprintf(latest_config, sizeof(latest_config),  WEBBOX_LIB_PATH "/playlists/.vlc_latest");
        if (latest_len >= sizeof(latest_config)) {
            latest_config[sizeof(latest_config)-1] = '\0';
            printf("VLC path: %s (%d)\n", latest_config, latest_len);
        }
        FILE *file = fopen (latest_config, "wt");
        if (file) {
            fprintf(file, "%s\n", playlists->folder);
            fclose (file);
        }        

    } else {
	if (vlc_pid != -1) {
	   printf("vlc already playing\n");
            folder = NULL;
	} else if (playlists) {
/*	    playlist *p = playlists;
	    while (p) {
	        printf("%s\n", p->folder);
	        p = p->more;
	    }*/
            folder = playlists->folder;
        } else {
            printf("No playlist. Create a link from %s to your music folder and give 'weston' rights.\n", WEBBOX_LIB_PATH "/playlists/");
            folder = NULL;
        }
    }

    bool success = false;
    if (folder) {
        success = vlc_command(cmd, folder);
    }  
    
    const char ok[] = "VLC playing...";
    const char nok[] = "Play failed!";
    const char *response = success ? ok : nok;
    if (write(sock, response, strlen(response)) != strlen(response)) {
        perror("send");
    }
//flush(sock);
    return true;
}

static bool cmd_stop(int sock, const char const *msg) {
    //const char cmd[] = "/stop";
printf("cmd_stop\n");
    bool success = vlc_command(msg, 0);

    const char ok[] = "Stopped.";
    const char nok[] = "Stop failed!";
    const char *response = success ? ok : nok;
    if (send(sock, response, strlen(response), 0) != strlen(response)) {
        perror("send");
    }
    return true;
}

static webbox_http_command commands[] = {
    { .name = "/load", .handle = cmd_load},
    //{ .name = "/playlist", .handle = cmd_playlist},
    { .name = "/play", .handle = cmd_play},
    { .name = "/stop", .handle = cmd_stop},
};

static bool command(int sock, const char const *path) {
    for (int i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
        if (strncmp(path, commands[i].name, strlen(commands[i].name)) == 0) {
            printf("vlc command: %s\n", path);
            commands[i].handle(sock, path);
            return true;
        }
    }
    return false;
}

static void command_init(void) {
    printf("vlc init\n");
    playlist_config_init();
    if (1) return;
    char buf[80];
    int len = snprintf(buf, sizeof(buf), WEBBOX_LIB_PATH "/playlists");
    if (len > sizeof(buf)) {
        buf[sizeof(buf)] = '\0';
        printf("VLC path: %s (%d)\n", buf, len);
        return;
    }


    DIR *d;
    struct dirent *dir;
    d = opendir(buf);
    if (d) {
      while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] != '.') {
            printf("%s\n", dir->d_name);
        }
      }
      closedir(d);
    }
}

static void command_exit(void) {
    printf("command_exit\n");
    /*playlist *p = playlists;
    while (p) {
        printf("remove %s\n", p->folder);
        free(p->folder);
        playlist *current = p;
        p = p->more;
        free(current);
    }*/
}

webbox_http_command webbox_http_vlc = {
    .name = "/VLC",
    .init = command_init,
    .handle = command,
    .exit = command_exit,
};
