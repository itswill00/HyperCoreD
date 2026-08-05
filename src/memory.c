
#include "memory.hpp"

void apply_memory_tuning(void) {
    sysfs_write("/proc/sys/vm/swappiness", "60");
    sysfs_write("/proc/sys/vm/dirty_ratio", "15");
    sysfs_write("/proc/sys/vm/dirty_background_ratio", "5");
    sysfs_write("/proc/sys/vm/vfs_cache_pressure", "100");
    sysfs_write("/proc/sys/vm/stat_interval", "5");
    sysfs_write("/proc/sys/vm/compaction_proactiveness", "20");
    sysfs_write("/proc/sys/vm/page-cluster", "0");
    sysfs_write("/proc/sys/vm/extfrag_threshold", "750");
}

void tune_memory_pressure(void) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return;

    int mem_free_kb = 0;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "MemAvailable:", 13) == 0 || strncmp(line, "MemFree:", 8) == 0) {
            char *p = line;
            while (*p && (*p < '0' || *p > '9')) p++;
            if (*p) {
                mem_free_kb = atoi(p);
                break;
            }
        }
    }
    fclose(f);

    if (mem_free_kb > 0 && mem_free_kb < 358400) {
        sysfs_write("/proc/sys/vm/swappiness", "80");
        sysfs_write("/proc/sys/vm/compaction_proactiveness", "50");
    } else {
        sysfs_write("/proc/sys/vm/swappiness", "60");
        sysfs_write("/proc/sys/vm/compaction_proactiveness", "20");
    }
}
