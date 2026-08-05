
#include "gpu.hpp"

void apply_gpu_tuning(void) {
    
    sysfs_write("/sys/kernel/fpsgo/fbt/switch_idleprefer", "1");
    sysfs_write("/sys/kernel/fpsgo/fbt/ultra_rescue", "1");
    sysfs_write("/sys/kernel/fpsgo/fbt/light_loading_policy", "10");
    sysfs_write("/sys/kernel/fpsgo/fbt/thrm_enable", "0");

    sysfs_write("/sys/module/ged/parameters/boost_gpu_enable", "0");
    sysfs_write("/sys/module/ged/parameters/enable_gpu_boost", "0");
    sysfs_write("/sys/module/ged/parameters/ged_smart_boost", "0");
    sysfs_write("/sys/module/ged/parameters/g_fb_dvfs_threshold", "15");
    sysfs_write("/sys/module/ged/parameters/gx_fb_dvfs_margin", "0");

    const char *game_mode_nodes[] = {
        "/sys/module/ged/parameters/gx_game_mode",
        "/sys/module/ged/parameters/gx_boost_on",
        NULL
    };
    sysfs_write_fallback(game_mode_nodes, "0");

    const char *devfreq_gov_nodes[] = {
        "/sys/class/devfreq/13000000.mali/governor",
        "/sys/devices/platform/soc/13000000.mali/devfreq/13000000.mali/governor",
        NULL
    };
    sysfs_write_fallback(devfreq_gov_nodes, "simple_ondemand");
    sysfs_write("/sys/class/devfreq/13000000.mali/polling_interval", "50");

    const char *devfreq_min_nodes[] = {
        "/sys/class/devfreq/13000000.mali/min_freq",
        "/sys/devices/platform/soc/13000000.mali/devfreq/13000000.mali/min_freq",
        NULL
    };
    sysfs_write_fallback(devfreq_min_nodes, "390000000");

    const char *devfreq_max_nodes[] = {
        "/sys/class/devfreq/13000000.mali/max_freq",
        "/sys/devices/platform/soc/13000000.mali/devfreq/13000000.mali/max_freq",
        NULL
    };
    sysfs_write_fallback(devfreq_max_nodes, "1003000000");

    sysfs_write("/sys/devices/platform/soc/13000000.mali/power_policy", "coarse_demand");
}
