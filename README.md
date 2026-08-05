# HyperCoreD

A lightweight native C system daemon and hardware tuner for MediaTek Helio G99 Ultra (MT6789) devices running Linux Kernel 5.10.x (mandatory). Developed by **[@itswill00](https://github.com/itswill00)** (Telegram: **@noticesa**). Built for Redmi Note 14 4G and compatible across MT6789 devices (Redmi Note 13 Pro 4G, Poco M6 Pro 4G, etc.).

This repository contains the standalone system daemon (`hypercored`), Android init service scripts, SELinux policy rules, and build blueprints for Custom ROM maintainers (AOSP, LineageOS) and ROM porters.

---

## Technical Overview & Operating Principles

HyperCoreD runs as a native C system process (`/vendor/bin/hypercored`) under root or system privileges. It interfaces directly with Linux kernel sysfs nodes, CPU frequency governors, Mali GPU devfreq drivers, MediaTek GED framework, and Android cgroups.

### Core Architecture

- **Foreground Game Detection**: Inspects `/dev/cpuset/top-app/cgroup.procs` to identify active PIDs in the top-app cgroup in under 0.1ms. If cgroup updating lags during window transitions, falls back to `dumpsys window` focus records.
- **MediaTek GED GPU DVFS**: Enforces `gpu_cust_boost_freq = 0` to prevent vendor daemons (e.g. Xiaomi Joyose) from forcing 1003MHz frequency spikes on light tasks. Configures `g_fb_dvfs_threshold = 20` and `gx_fb_dvfs_margin = 15`, allowing smooth 38-step OPP scaling (390MHz - 1003MHz).
- **Battery Thermal & Charging Guard**: Evaluates battery temperature during gaming sessions and adjusts charge current limits dynamically (>=42°C -> 2A, 37°C-41°C -> 4A, <37°C -> 8A). Automatically detects microampere (uA) vs milliampere (mA) unit scales across custom kernels.
- **Sub-Cycle Battery Tracker**: Accumulates partial depth-of-discharge (DOD) progress in `/data/adb/modules/tanzanite_hypercore/cycle_tracker.conf` across reboots. Increments reported battery cycles live when cumulative charging reaches 100%, bypassing delayed hardware EEPROM flushes.
- **IPC Unix Socket**: Exposes a local socket at `/dev/socket/hypercore.sock` for status queries (`GET_STATUS`) and WebUI control.

---

## Edge Case Handling & System Resilience

1. **Boot Race Conditions**: If sysfs nodes (e.g., Mali GPU devfreq or power supply nodes) are not ready during early boot, `hypercored` enters a non-blocking retry loop for up to 30 seconds before declaring node absence.
2. **OOM & LMK Protection**: Configured with `oom_score_adjust -1000` in init service to prevent Android Low Memory Killer from terminating the daemon under memory pressure.
3. **Signal Safety & Baseline Restoration**: Signal handlers (`SIGTERM`, `SIGINT`, `SIGHUP`) capture termination events and restore original CPU governors, Mali power policies, and charge limits saved during startup.
4. **Daemon Tampering Recovery**: Periodically verifies sysfs node state every 1-2 seconds. If vendor thermal daemons (`mi_thermald`, `joyose`) alter GPU ceilings or governors, `hypercored` re-enforces profile settings automatically.

---

## Kernel Requirements & Safety Constraints

- **Kernel Version**: Requires Linux Kernel 5.10.x (mandatory). Necessary for sysfs node structure, Mali-G57 devfreq paths, and MediaTek GED parameters.
- **System-Background Cores**: `system-background` cpuset must never be restricted below Little cores `0-3`. Hardware HALs (Audio, Sensor, SurfaceFlinger helper threads) execute in system-background; restricting them causes Binder starvation and Watchdog soft reboots.
- **Memory Compaction**: Relies on kernel `kcompactd` (`compaction_proactiveness = 20`). Synchronous writes to `/proc/sys/vm/compact_memory` are avoided to prevent kernel `mmap_lock` contention across zones.
- **Thermal Mode Coexistence**: In `PROFILE_Gaming`, writes `"10"` to `/sys/class/thermal/thermal_message/sconfig` (raising Xiaomi thermal throttle limits to 55°C without display dimming). Reverts to `"0"` in Interactive and Sleep profiles.

---

## Integration Guide

### AOSP Source Tree Integration

Clone into your ROM source tree under `device/mediatek/mt6789-common/HyperCoreD`:

```bash
git clone https://github.com/itswill00/HyperCoreD device/mediatek/mt6789-common/HyperCoreD
```

In your `device.mk` or `device_common.mk`, add `hypercored` to `PRODUCT_PACKAGES`:

```makefile
PRODUCT_PACKAGES += \
    hypercored
```

Build with `m hypercored` or `mka bacon`.

### Flashable ROM Port Deployment

1. Copy `hypercored` to `/vendor/bin/hypercored` and set permissions to `0755`.
2. Copy `init.hypercore.rc` to `/vendor/etc/init/hw/init.hypercore.rc`.
3. Append this line to your vendor `init.target.rc`:

```rc
import /vendor/etc/init/hw/init.hypercore.rc
```

---

## Android Init Service Configuration (`init.hypercore.rc`)

```rc
on early-boot
    chmod 0664 /sys/class/devfreq/13000000.mali/governor
    chmod 0664 /sys/class/devfreq/13000000.mali/polling_interval
    chmod 0664 /sys/class/devfreq/13000000.mali/min_freq
    chmod 0664 /sys/class/devfreq/13000000.mali/max_freq
    chmod 0664 /sys/devices/platform/soc/13000000.mali/power_policy
    chmod 0664 /sys/module/ged/parameters/g_fb_dvfs_threshold
    chmod 0664 /sys/module/ged/parameters/gx_fb_dvfs_margin
    chmod 0664 /sys/module/ged/parameters/gpu_cust_boost_freq
    chmod 0664 /sys/module/ged/parameters/gpu_cust_upbound_freq
    chmod 0664 /sys/module/ged/parameters/gpu_bottom_freq
    chmod 0664 /sys/module/ged/parameters/boost_gpu_enable
    chmod 0664 /sys/module/ged/parameters/ged_smart_boost
    chmod 0664 /sys/module/ged/parameters/ged_boost_enable
    chmod 0664 /sys/module/ged/parameters/enable_gpu_boost
    chown root system /sys/class/devfreq/13000000.mali/governor
    chown root system /sys/class/devfreq/13000000.mali/polling_interval

service hypercored /vendor/bin/hypercored
    class main
    user root
    group root system readproc
    capabilities SYS_NICE SYS_RESOURCE
    writepid /dev/cpuset/top-app/tasks
    oom_score_adjust -1000
    onrestart restart hypercored
    seclabel u:r:su:s0
```

---

## SELinux Policy (`sepolicy/hypercore.te` & `sepolicy/file_contexts`)

Designed to comply with AOSP Google CTS `neverallow` rules (Android 12, 13, 14, 15).

`sepolicy/hypercore.te`:
```pp
type hypercored, domain;
type hypercored_exec, exec_type, vendor_file_type, file_type;

init_daemon_domain(hypercored)

allow hypercored sysfs_devices_system_cpu:file rw_file_perms;
allow hypercored sysfs_devices_system_cpu:dir r_dir_perms;
allow hypercored sysfs_gpu:file rw_file_perms;
allow hypercored sysfs_gpu:dir r_dir_perms;
allow hypercored sysfs_batteryinfo:file rw_file_perms;
allow hypercored sysfs_batteryinfo:dir r_dir_perms;
allow hypercored sysfs_thermal:file rw_file_perms;
allow hypercored sysfs_thermal:dir r_dir_perms;

allow hypercored cgroup:file rw_file_perms;
allow hypercored cgroup:dir r_dir_perms;
allow hypercored cgroup_v2:file rw_file_perms;
allow hypercored cgroup_v2:dir r_dir_perms;

allow hypercored proc:file r_file_perms;
allow hypercored proc_stat:file r_file_perms;
allow hypercored proc_pid_max:file r_file_perms;

type hypercored_socket, file_type, coredomain_socket;
allow hypercored hypercored_socket:sock_file create_file_perms;
allow hypercored self:capability { sys_nice };
binder_use(hypercored)
```

`sepolicy/file_contexts`:
```file_contexts
/vendor/bin/hypercored    u:object_r:hypercored_exec:s0
/dev/socket/hypercore.sock u:object_r:hypercored_socket:s0
```

---

## IPC Socket API (`GET_STATUS`)

Send `GET_STATUS` to `/dev/socket/hypercore.sock` to retrieve JSON status telemetry:

```json
{
  "status": "ok",
  "pid": 20265,
  "profile": "Interactive",
  "thermal_tier": 0,
  "cpu_temp": 46,
  "bat_temp": 38,
  "gpu_temp": 44,
  "chg_temp": 39,
  "is_charging": 0,
  "gpu_load": 0,
  "battery_cycles": 347,
  "uptime_sec": 120,
  "bat_health": "Good",
  "bat_status": "Discharging",
  "bat_tech": "Li-poly"
}
```

---

## Developer & License

- **Developer**: [@itswill](https://github.com/itswill00)
- **License**: MIT License - open for integration into Custom ROMs and ROM Port builds.
