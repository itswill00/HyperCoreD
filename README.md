# HyperCoreD

A lightweight native C system daemon and hardware tuner for MediaTek Helio G99 Ultra (MT6789) devices running Linux Kernel 5.10.x (mandatory), developed by **[@itswill](https://github.com/itswill00)**. Built for Redmi Note 14 4G and compatible with all MT6789 devices (Redmi Note 13 Pro 4G, Poco M6 Pro 4G, etc.).

This repository contains the standalone system daemon (`hypercored`), Android init service scripts, SELinux policy rules, and build blueprints for Custom ROM maintainers (AOSP, LineageOS) and ROM porters.

---

## What It Does

- **Game Detection**: Scans `/dev/cpuset/top-app/cgroup.procs` to find foreground games in under 0.1ms. If cgroup state lags, falls back to `dumpsys window` focus records.
- **GPU Scaling (MediaTek GED)**: Keeps `gpu_cust_boost_freq = 0` so Joyose can't force 1003MHz spikes on light tasks. Sets threshold to 20 and margin to 15, letting the GPU scale smoothly across all 38 OPP steps (390MHz - 1003MHz).
- **Thermal & Charging Guard**: Adjusts battery charging current based on temperature when gaming (>=42°C -> 2A, 37-41°C -> 4A, <37°C -> 8A).
- **Battery Cycle Tracker**: Tracks partial charges across reboots (`cycle_tracker.conf`) and increments cycle count live when cumulative charging hits 100%, without waiting days for kernel EEPROM flushes.
- **IPC Interface**: Exposes a Unix socket at `/dev/socket/hypercore.sock` for WebUI or app control.

---

## MT6789 Kernel & Safety Rules

0. **Kernel Version**: Requires Linux Kernel 5.10.x (mandatory). Necessary for sysfs node compatibility, Mali GPU devfreq paths, and MediaTek GED DVFS parameters.
1. **System-Background Cores**: Never restrict `system-background` cpuset below cores `0-3`. HAL threads like Audio, Sensor, and SurfaceFlinger helpers run here; restricting them triggers Binder timeouts and soft reboots.
2. **Compaction**: Uses `kcompactd` (`compaction_proactiveness = 20`). Never write "1" synchronously to `/proc/sys/vm/compact_memory` as it locks kernel `mmap_lock` and freezes threads in D-state.
3. **Xiaomi Thermal Profiles**: In `PROFILE_Gaming`, writes `"10"` to `/sys/class/thermal/thermal_message/sconfig` to raise thermal limits to 55°C without dimming the screen. Reverts to `"0"` in Interactive and Sleep profiles.

---

## Adding to an AOSP ROM Build

Clone this repository into your ROM source tree under `device/mediatek/mt6789-common/HyperCoreD`:

```bash
git clone https://github.com/itswill00/HyperCoreD device/mediatek/mt6789-common/HyperCoreD
```

In your `device.mk` or `device_common.mk`, add `hypercored` to `PRODUCT_PACKAGES`:

```makefile
PRODUCT_PACKAGES += \
    hypercored
```

Build normally with `m hypercored` or `mka bacon`.

---

## Adding to a Flashable ROM Port

1. Copy `hypercored` to `/vendor/bin/hypercored` and `chmod 0755`.
2. Copy `init.hypercore.rc` to `/vendor/etc/init/hw/init.hypercore.rc`.
3. Add this line to your vendor `init.target.rc`:

```rc
import /vendor/etc/init/hw/init.hypercore.rc
```

---

## Android Init Config (`init.hypercore.rc`)

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
    onrestart restart hypercored
    seclabel u:r:su:s0
```

---

## SELinux Policy (`sepolicy/hypercore.te`)

```pp
type hypercored, domain;
type hypercored_exec, exec_type, vendor_file_type, file_type;

init_daemon_domain(hypercored)

allow hypercored sysfs_devices_system_cpu:file rw_file_perms;
allow hypercored sysfs_gpu:file rw_file_perms;
allow hypercored thermal_zone:file rw_file_perms;
allow hypercored cgroup:file rw_file_perms;
allow hypercored proc_type:file rw_file_perms;
```

---

## IPC Socket (`GET_STATUS`)

Send `GET_STATUS` to `/dev/socket/hypercore.sock` to get JSON status output:

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
- **License**: MIT License - feel free to include this in your Custom ROM or ROM Port builds.
