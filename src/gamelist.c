
#include "gamelist.hpp"

#define MAX_GAMES 256
#define PKG_NAME_LEN 128

static char      s_games[MAX_GAMES][PKG_NAME_LEN];
static profile_t s_profiles[MAX_GAMES];
static int       s_game_count = 0;
static int       s_inotify_fd = -1;



void load_gamelist(void) {
    s_game_count = 0;
    char path[256];
    snprintf(path, sizeof(path), "%s/gamelist.txt", g_nodes.mod_dir);

    FILE *f = fopen(path, "r");
    if (!f) {
        f = fopen("/sdcard/Android/gamelist.txt", "r");
    }
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f) && s_game_count < MAX_GAMES) {
        
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' ')) {
            line[--len] = '\0';
        }
        
        if (line[0] == '#' || line[0] == '\0') continue;

        char *colon = strchr(line, ':');
        profile_t prof = PROFILE_Gaming;

        if (colon) {
            *colon = '\0';
            char *prof_str = colon + 1;
            if (strcmp(prof_str, "INTERACTIVE") == 0 || strcmp(prof_str, "BALANCED") == 0) prof = PROFILE_Interactive;
            else if (strcmp(prof_str, "SLEEP") == 0 || strcmp(prof_str, "SAVER") == 0) prof = PROFILE_Sleep;
            else if (strcmp(prof_str, "GAMING") == 0) prof = PROFILE_Gaming;
        }

        strncpy(s_games[s_game_count], line, PKG_NAME_LEN - 1);
        s_games[s_game_count][PKG_NAME_LEN - 1] = '\0';
        s_profiles[s_game_count] = prof;
        s_game_count++;
    }
    fclose(f);

    if (s_game_count == 0) {
        FILE *pp = popen("pm list packages -3 2>/dev/null | cut -d: -f2 | grep -iE 'game|legend|pubg|mihoyo|genshin|honkai|freefire|roblox|activision|shooter|mojang|minecraft|supercell|brawl|clash|garena|stumble|pokemon|wanda|maleo|konami|krafton|netmarble|nexon|ea\\.gp|riotgames|square_enix|bandainamco|gameloft|zynga|rovio|miniclip|yostar|ubisoft|subway|bussimulator|carx|slither|angrybirds|asphalt|shadowfight|realracing|needforspeed|efootball|pes20|fifa|tft|nintendo|sega|squareenix|capcom'", "r");
        if (pp) {
            char pkg_buf[128];
            FILE *fw = fopen(path, "a");
            while (fgets(pkg_buf, sizeof(pkg_buf), pp) && s_game_count < MAX_GAMES) {
                size_t len = strlen(pkg_buf);
                while (len > 0 && (pkg_buf[len - 1] == '\r' || pkg_buf[len - 1] == '\n' || pkg_buf[len - 1] == ' ')) {
                    pkg_buf[--len] = '\0';
                }
                if (pkg_buf[0] == '\0') continue;

                strncpy(s_games[s_game_count], pkg_buf, PKG_NAME_LEN - 1);
                s_games[s_game_count][PKG_NAME_LEN - 1] = '\0';
                s_profiles[s_game_count] = PROFILE_Gaming;
                s_game_count++;

                if (fw) {
                    fprintf(fw, "%s:GAMING\n", pkg_buf);
                }
            }
            if (fw) fclose(fw);
            pclose(pp);
        }
    }
}

void init_gamelist_watcher(void) {
    s_inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (s_inotify_fd < 0) return;

    char path[256];
    snprintf(path, sizeof(path), "%s/gamelist.txt", g_nodes.mod_dir);
    inotify_add_watch(s_inotify_fd, path, IN_MODIFY | IN_CLOSE_WRITE);

    inotify_add_watch(s_inotify_fd, "/sdcard/Android/gamelist.txt", IN_MODIFY | IN_CLOSE_WRITE);
}

void check_gamelist_inotify(void) {
    if (s_inotify_fd < 0) return;

    char buf[512];
    ssize_t len = read(s_inotify_fd, buf, sizeof(buf));
    if (len > 0) {
        load_gamelist();
    }
}

int is_game_in_foreground(char *out_game_name, size_t max_len, profile_t *out_profile) {
    if (out_game_name && max_len > 0) out_game_name[0] = '\0';
    if (out_profile) *out_profile = PROFILE_Gaming;

    check_gamelist_inotify();

    if (s_game_count == 0) {
        load_gamelist();
    }

    if (s_game_count == 0) {
        return 0;
    }

    const char *procs_paths[] = {
        "/dev/cpuset/top-app/cgroup.procs",
        "/sys/fs/cgroup/top-app/cgroup.procs",
        "/dev/cpuset/top-app/tasks",
        NULL
    };

    for (int p = 0; procs_paths[p]; p++) {
        FILE *fp = fopen(procs_paths[p], "r");
        if (!fp) continue;

        char line_str[32];
        int read_count = 0;

        while (fgets(line_str, sizeof(line_str), fp) && read_count < 250) {
            int pid = atoi(line_str);
            if (pid <= 0) continue;
            read_count++;

            char cmdpath[64];
            snprintf(cmdpath, sizeof(cmdpath), "/proc/%d/cmdline", pid);
            int fd = open(cmdpath, O_RDONLY | O_CLOEXEC);
            if (fd < 0) continue;

            char cmdline[256];
            ssize_t n = read(fd, cmdline, sizeof(cmdline) - 1);
            close(fd);

            if (n <= 0) continue;
            cmdline[n] = '\0';

            for (int i = 0; i < s_game_count; i++) {
                size_t pkg_len = strlen(s_games[i]);
                if (pkg_len == 0) continue;

                if (strncmp(cmdline, s_games[i], pkg_len) == 0 &&
                    (cmdline[pkg_len] == '\0' || cmdline[pkg_len] == ':' || cmdline[pkg_len] == ' ')) {
                    fclose(fp);

                    if (out_game_name && max_len > 0) {
                        strncpy(out_game_name, s_games[i], max_len - 1);
                        out_game_name[max_len - 1] = '\0';
                    }
                    if (out_profile) {
                        *out_profile = s_profiles[i];
                    }
                    return 1;
                }
            }
        }
        fclose(fp);
    }

    FILE *pp = popen("dumpsys window 2>/dev/null | grep -E 'mCurrentFocus|mFocusedApp'", "r");
    if (pp) {
        char dump_buf[512];
        while (fgets(dump_buf, sizeof(dump_buf), pp)) {
            for (int i = 0; i < s_game_count; i++) {
                if (s_games[i][0] != '\0' && strstr(dump_buf, s_games[i]) != NULL) {
                    pclose(pp);
                    if (out_game_name && max_len > 0) {
                        strncpy(out_game_name, s_games[i], max_len - 1);
                        out_game_name[max_len - 1] = '\0';
                    }
                    if (out_profile) {
                        *out_profile = s_profiles[i];
                    }
                    return 1;
                }
            }
        }
        pclose(pp);
    }

    return 0;
}
