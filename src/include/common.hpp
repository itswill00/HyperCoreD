
#ifndef HYPERCORE_COMMON_HPP
#define HYPERCORE_COMMON_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/resource.h>

#define LOG_PATH        "/sdcard/Android/hypercore.log"
#define FREQ_LITTLE_MAX 2000000
#define FREQ_BIG_MAX    2200000

typedef enum {
    PROFILE_Sleep,
    PROFILE_Interactive,
    PROFILE_Gaming
} profile_t;

extern const char *g_profile_names[];

struct hw_nodes {
    char mod_dir[256];
    char pid_file[256];
    char lock_file[256];
    char cpu_temp[256];
    char bat_temp[256];
    char bat_status[256];
    char backlight[256];
    char touch_thp_smooth[256];
    char touch_edge[256];
    char charge_control[256];
    int  has_sugov_ext;
    int  has_schedutil;
};

struct core_state {
    profile_t current_profile;
    int thermal_tier;
    int thermal_hold_ticks;
    int touch_boost_ticks;
    int gaming_hold_ticks;
    int launch_boost_ticks;
    int app_boost_ticks;
    int prev_load;
    int is_charging;
};

extern volatile sig_atomic_t g_running;
extern struct hw_nodes g_nodes;
extern struct core_state g_state;

#endif 
