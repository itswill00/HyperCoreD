
#include "thermal.hpp"
#include "sysfs.hpp"
#include "log.hpp"

void scan_thermal_zones(void) {
    DIR *d = opendir("/sys/class/thermal");
    if (d) {
        struct dirent *ent;
        while ((ent = readdir(d)) != NULL) {
            if (strncmp(ent->d_name, "thermal_zone", 12) != 0) continue;

            char path[256];
            snprintf(path, sizeof(path), "/sys/class/thermal/%s/type", ent->d_name);
            int fd = open(path, O_RDONLY | O_CLOEXEC);
            if (fd < 0) continue;

            char type[64];
            ssize_t n = read(fd, type, sizeof(type) - 1);
            close(fd);

            if (n <= 0) continue;
            type[n] = '\0';

            char temp_path[256];
            snprintf(temp_path, sizeof(temp_path), "/sys/class/thermal/%s/temp", ent->d_name);
            int val = sysfs_read_int(temp_path);

            if (val > 0) {
                if (g_nodes.cpu_temp[0] == '\0' && (strstr(type, "cpu") || strstr(type, "soc") || strstr(type, "CPU"))) {
                    snprintf(g_nodes.cpu_temp, sizeof(g_nodes.cpu_temp), "%s", temp_path);
                }
                if (g_nodes.bat_temp[0] == '\0' && (strstr(type, "bat") || strstr(type, "battery"))) {
                    snprintf(g_nodes.bat_temp, sizeof(g_nodes.bat_temp), "%s", temp_path);
                }
            }
        }
        closedir(d);
    }

    if (g_nodes.cpu_temp[0] == '\0') {
        strcpy(g_nodes.cpu_temp, access("/sys/class/thermal/thermal_zone16/temp", F_OK) == 0 ?
               "/sys/class/thermal/thermal_zone16/temp" : "/sys/class/thermal/thermal_zone0/temp");
    }
    if (g_nodes.bat_temp[0] == '\0') {
        strcpy(g_nodes.bat_temp, access("/sys/class/thermal/thermal_zone25/temp", F_OK) == 0 ?
               "/sys/class/thermal/thermal_zone25/temp" : "/sys/class/thermal/thermal_zone1/temp");
    }

    if (access("/sys/class/power_supply/battery/status", F_OK) == 0) {
        strcpy(g_nodes.bat_status, "/sys/class/power_supply/battery/status");
    } else {
        const char *st_paths[] = {
            "/sys/class/power_supply/bms/status",
            "/sys/class/power_supply/usb/status",
            "/sys/class/power_supply/main/status",
            NULL
        };
        for (int i = 0; st_paths[i]; i++) {
            if (access(st_paths[i], F_OK) == 0) {
                strcpy(g_nodes.bat_status, st_paths[i]);
                break;
            }
        }
    }
}

int check_charging_status(void) {
    if (g_nodes.bat_status[0] == '\0') return 0;
    int fd = open(g_nodes.bat_status, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return 0;
    char buf[32];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return 0;
    buf[n] = '\0';
    return (strstr(buf, "Charging") != NULL);
}

int calculate_thermal_tier(int cpu_temp, int bat_temp) {
    int tier = g_state.thermal_tier;
    int is_chg = g_state.is_charging;

    int t3_cpu = is_chg ? 72 : 75;
    int t3_bat = is_chg ? 50 : 53;

    if (cpu_temp >= t3_cpu || bat_temp >= t3_bat) {
        tier = 3;
        g_state.thermal_hold_ticks = 3;
    } else if (g_state.thermal_hold_ticks > 0) {
        g_state.thermal_hold_ticks--;
        tier = 3;
    } else if (g_state.thermal_tier == 3) {
        if (cpu_temp <= 64 && bat_temp <= 46) {
            tier = (cpu_temp >= 58 || bat_temp >= 44) ? 2 : 1;
        }
    } else if (cpu_temp >= 65 || bat_temp >= 48) {
        tier = 2;
    } else if (g_state.thermal_tier == 2) {
        if (cpu_temp < 58 && bat_temp < 44) {
            tier = (cpu_temp >= 54 || bat_temp >= 42) ? 1 : 0;
        }
    } else if (g_state.thermal_tier == 1) {
        if (cpu_temp < 52 && bat_temp < 40) {
            tier = 0;
        }
    } else if (cpu_temp >= 58 || bat_temp >= 44) {
        tier = 1;
    } else {
        tier = 0;
    }

    return tier;
}

static float s_accumulated_pct = 0.0f;
static int   s_prev_cap_for_cycle = -1;
static int   s_added_cycles = 0;
static int   s_tracker_loaded = 0;

static void load_cycle_tracker(void) {
    if (s_tracker_loaded) return;
    char path[256];
    snprintf(path, sizeof(path), "%s/cycle_tracker.conf", g_nodes.mod_dir);
    FILE *f = fopen(path, "r");
    if (f) {
        if (fscanf(f, "%d %f", &s_added_cycles, &s_accumulated_pct) != 2) {
            s_added_cycles = 0;
            s_accumulated_pct = 0.0f;
        }
        fclose(f);
    }
    s_tracker_loaded = 1;
}

static void save_cycle_tracker(void) {
    char path[256];
    snprintf(path, sizeof(path), "%s/cycle_tracker.conf", g_nodes.mod_dir);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%d %.2f\n", s_added_cycles, s_accumulated_pct);
    fclose(f);
}

void track_charging_cycles(int is_charging) {
    load_cycle_tracker();

    int current_cap = sysfs_read_int("/sys/class/power_supply/battery/capacity");
    if (current_cap <= 0) {
        current_cap = sysfs_read_int("/sys/class/power_supply/bms/capacity");
    }

    if (is_charging && s_prev_cap_for_cycle > 0 && current_cap > s_prev_cap_for_cycle) {
        int delta = current_cap - s_prev_cap_for_cycle;
        if (delta > 0 && delta <= 50) {
            s_accumulated_pct += (float)delta;
            if (s_accumulated_pct >= 100.0f) {
                int new_added = (int)(s_accumulated_pct / 100.0f);
                s_added_cycles += new_added;
                s_accumulated_pct -= (float)(new_added * 100);
                log_info("Battery", "Dynamic full charge cycle reached! Total added cycles: %d", s_added_cycles);
            }
            save_cycle_tracker();
        }
    }
    s_prev_cap_for_cycle = current_cap;
}

int get_true_battery_cycles(void) {
    load_cycle_tracker();

    int base_cycles = 0;
    int cycle_count = sysfs_read_int("/sys/class/power_supply/battery/cycle_count");
    if (cycle_count > 0) {
        base_cycles = (cycle_count > 1000) ? (cycle_count / 16) : cycle_count;
    } else {
        int fg_raw = sysfs_read_int("/sys/class/power_supply/battery/fg1_cycle");
        if (fg_raw > 0) {
            base_cycles = (fg_raw > 1000) ? (fg_raw / 16) : fg_raw;
        } else {
            int auth_cycles = sysfs_read_int("/sys/class/power_supply/battery/auth_dev_batt_cycle");
            if (auth_cycles > 0) {
                base_cycles = (auth_cycles > 1000) ? (auth_cycles / 16) : auth_cycles;
            } else {
                int bms_cycles = sysfs_read_int("/sys/class/power_supply/bms/cycle_count");
                if (bms_cycles > 0) {
                    base_cycles = (bms_cycles > 1000) ? (bms_cycles / 16) : bms_cycles;
                }
            }
        }
    }

    return base_cycles + s_added_cycles;
}

void fix_battery_cycle_count(void) {
    int true_cycles = get_true_battery_cycles();
    if (true_cycles <= 0) return;

    int current_cycles = sysfs_read_int("/sys/class/power_supply/battery/cycle_count");
    if (current_cycles != true_cycles) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", true_cycles);

        chmod("/sys/class/power_supply/battery/cycle_count", 0664);
        sysfs_write("/sys/class/power_supply/battery/cycle_count", buf);

        chmod("/sys/class/power_supply/battery/auth_dev_batt_cycle", 0664);
        sysfs_write("/sys/class/power_supply/battery/auth_dev_batt_cycle", buf);
        log_info("Battery", "Battery cycle count calibrated to %d cycles", true_cycles);
    }
}



