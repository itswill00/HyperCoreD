
#include "io.hpp"

void set_read_ahead(const char *kb_val) {
    DIR *d = opendir("/sys/block");
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "sd", 2) == 0 ||
            strncmp(ent->d_name, "mmcblk", 6) == 0 ||
            strncmp(ent->d_name, "dm-", 3) == 0 ||
            strncmp(ent->d_name, "ufs", 3) == 0) {
            char p[256];
            snprintf(p, sizeof(p), "/sys/block/%s/queue/read_ahead_kb", ent->d_name);
            sysfs_write(p, kb_val);
        }
    }
    closedir(d);
}

void set_io_nr_requests(const char *nr_str) {
    DIR *d = opendir("/sys/block");
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "sd", 2) == 0 ||
            strncmp(ent->d_name, "mmcblk", 6) == 0 ||
            strncmp(ent->d_name, "dm-", 3) == 0 ||
            strncmp(ent->d_name, "ufs", 3) == 0) {
            char p[256];
            snprintf(p, sizeof(p), "/sys/block/%s/queue/nr_requests", ent->d_name);
            sysfs_write(p, nr_str);
        }
    }
    closedir(d);
}

void apply_io_tuning(void) {
    DIR *d = opendir("/sys/block");
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "sd", 2) == 0 ||
            strncmp(ent->d_name, "mmcblk", 6) == 0 ||
            strncmp(ent->d_name, "dm-", 3) == 0 ||
            strncmp(ent->d_name, "ufs", 3) == 0) {
            char p[256];
            snprintf(p, sizeof(p), "/sys/block/%s/queue/scheduler", ent->d_name);
            sysfs_write(p, "none");
            snprintf(p, sizeof(p), "/sys/block/%s/queue/read_ahead_kb", ent->d_name);
            sysfs_write(p, "256");
            snprintf(p, sizeof(p), "/sys/block/%s/queue/iostats", ent->d_name);
            sysfs_write(p, "0");
            snprintf(p, sizeof(p), "/sys/block/%s/queue/nr_requests", ent->d_name);
            sysfs_write(p, "64");
            snprintf(p, sizeof(p), "/sys/block/%s/queue/nomerges", ent->d_name);
            sysfs_write(p, "2");
            snprintf(p, sizeof(p), "/sys/block/%s/queue/add_random", ent->d_name);
            sysfs_write(p, "0");
            snprintf(p, sizeof(p), "/sys/block/%s/queue/rq_affinity", ent->d_name);
            sysfs_write(p, "2");
        }
    }
    closedir(d);
}

void apply_irq_tuning(void) {
    DIR *dir = opendir("/proc/irq");
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        int irq = atoi(ent->d_name);
        if (irq <= 0) continue;

        char irq_dir_path[256];
        snprintf(irq_dir_path, sizeof(irq_dir_path), "/proc/irq/%d", irq);
        DIR *subdir = opendir(irq_dir_path);
        if (!subdir) continue;

        struct dirent *subent;
        int matched = 0;
        while ((subent = readdir(subdir)) != NULL) {
            if (subent->d_name[0] != '.') {
                const char *name = subent->d_name;
                if (strstr(name, "mali") || strstr(name, "ged") || strstr(name, "kgsl") ||
                    strstr(name, "dsi") || strstr(name, "touch") || strstr(name, "tpd") ||
                    strstr(name, "fts") || strstr(name, "GPU") || strstr(name, "gpu")) {
                    matched = 1;
                    break;
                }
            }
        }
        closedir(subdir);

        if (matched) {
            char aff_path[256];
            snprintf(aff_path, sizeof(aff_path), "/proc/irq/%d/smp_affinity", irq);
            sysfs_write(aff_path, "c0");
        }
    }
    closedir(dir);
}
