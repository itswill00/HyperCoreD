
#ifndef HYPERCORE_CPU_HPP
#define HYPERCORE_CPU_HPP

#include "common.hpp"
#include "sysfs.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void set_rate_limits(const char *up, const char *down);
void apply_cpuset(void);
void apply_cgroup_gaming_policy(int enable);
void set_cpu_governor(const char *gov);
void set_cpu_freqs(int min_lit, int max_lit, int min_big, int max_big, const char *up_rate, const char *down_rate);
void apply_profile(profile_t prof, int tier, int gpu_load);

#ifdef __cplusplus
}
#endif

#endif 
