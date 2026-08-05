
#include "sysfs.hpp"

void sysfs_write(const char *path, const char *val) {
    if (!path || path[0] == '\0' || !val) return;
    int fd = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) return;
    write(fd, val, strlen(val));
    close(fd);
}

void sysfs_write_fallback(const char *paths[], const char *val) {
    if (!paths || !val) return;
    for (int i = 0; paths[i]; i++) {
        if (paths[i][0] != '\0' && access(paths[i], W_OK) == 0) {
            sysfs_write(paths[i], val);
            break;
        }
    }
}

int sysfs_read_int(const char *path) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return 0;
    char buf[32];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return 0;
    buf[n] = '\0';
    return atoi(buf);
}

int sysfs_read_str(const char *path, char *out_buf, size_t max_len) {
    if (!out_buf || max_len == 0) return 0;
    out_buf[0] = '\0';

    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return 0;
    ssize_t n = read(fd, out_buf, max_len - 1);
    close(fd);
    if (n <= 0) return 0;
    out_buf[n] = '\0';

    size_t len = strlen(out_buf);
    while (len > 0 && (out_buf[len - 1] == '\r' || out_buf[len - 1] == '\n' || out_buf[len - 1] == ' ')) {
        out_buf[--len] = '\0';
    }
    return 1;
}

static struct {
    char gov0[64];
    char gov6[64];
    char mali_policy[64];
    char mali_gpu_gov[64];
    char mali_poll_int[64];
    char migration_cost[64];
    char charge_limit[64];
    int  has_baseline;
} s_baseline = { "", "", "", "", "", "", "", 0 };

void save_baseline_nodes(void) {
    if (s_baseline.has_baseline) return;

    sysfs_read_str("/sys/devices/system/cpu/cpufreq/policy0/scaling_governor", s_baseline.gov0, sizeof(s_baseline.gov0));
    sysfs_read_str("/sys/devices/system/cpu/cpufreq/policy6/scaling_governor", s_baseline.gov6, sizeof(s_baseline.gov6));
    sysfs_read_str("/sys/devices/platform/soc/13000000.mali/power_policy", s_baseline.mali_policy, sizeof(s_baseline.mali_policy));
    sysfs_read_str("/sys/class/devfreq/13000000.mali/governor", s_baseline.mali_gpu_gov, sizeof(s_baseline.mali_gpu_gov));
    sysfs_read_str("/sys/class/devfreq/13000000.mali/polling_interval", s_baseline.mali_poll_int, sizeof(s_baseline.mali_poll_int));
    sysfs_read_str("/proc/sys/kernel/sched_migration_cost_ns", s_baseline.migration_cost, sizeof(s_baseline.migration_cost));
    sysfs_read_str("/sys/class/power_supply/battery/constant_charge_current_max", s_baseline.charge_limit, sizeof(s_baseline.charge_limit));

    s_baseline.has_baseline = 1;
}

void restore_baseline_nodes(void) {
    if (!s_baseline.has_baseline) return;

    if (s_baseline.gov0[0] != '\0') sysfs_write("/sys/devices/system/cpu/cpufreq/policy0/scaling_governor", s_baseline.gov0);
    if (s_baseline.gov6[0] != '\0') sysfs_write("/sys/devices/system/cpu/cpufreq/policy6/scaling_governor", s_baseline.gov6);
    if (s_baseline.mali_policy[0] != '\0') sysfs_write("/sys/devices/platform/soc/13000000.mali/power_policy", s_baseline.mali_policy);
    if (s_baseline.mali_gpu_gov[0] != '\0') sysfs_write("/sys/class/devfreq/13000000.mali/governor", s_baseline.mali_gpu_gov);
    if (s_baseline.mali_poll_int[0] != '\0' && atoi(s_baseline.mali_poll_int) > 0) {
        sysfs_write("/sys/class/devfreq/13000000.mali/polling_interval", s_baseline.mali_poll_int);
    } else {
        sysfs_write("/sys/class/devfreq/13000000.mali/polling_interval", "50");
    }
    if (s_baseline.migration_cost[0] != '\0') sysfs_write("/proc/sys/kernel/sched_migration_cost_ns", s_baseline.migration_cost);
    if (s_baseline.charge_limit[0] != '\0') sysfs_write("/sys/class/power_supply/battery/constant_charge_current_max", s_baseline.charge_limit);
}

void update_module_prop_status(const char *status) {
    static char s_last_status[128] = "";
    static time_t s_last_write_time = 0;
    time_t now = time(NULL);

    if (status && strcmp(s_last_status, status) == 0 && (now - s_last_write_time) < 30) {
        return;
    }

    if (status) {
        strncpy(s_last_status, status, sizeof(s_last_status) - 1);
        s_last_status[sizeof(s_last_status) - 1] = '\0';
    }
    s_last_write_time = now;

    char prop_path[256];
    snprintf(prop_path, sizeof(prop_path), "%s/module.prop", g_nodes.mod_dir);

    FILE *f = fopen(prop_path, "r");
    if (!f) return;

    char lines[32][256];
    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), f) && count < 32) {
        count++;
    }
    fclose(f);

    f = fopen(prop_path, "w");
    if (!f) return;

    for (int i = 0; i < count; i++) {
        if (strncmp(lines[i], "description=", 12) == 0) {
            fprintf(f, "description=[Active: %s] Smart kernel optimizer & gaming daemon for Helio G99 Ultra.\n", status);
        } else {
            fputs(lines[i], f);
        }
    }
    fclose(f);
}
