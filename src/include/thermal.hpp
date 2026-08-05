
#ifndef HYPERCORE_THERMAL_HPP
#define HYPERCORE_THERMAL_HPP

#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void scan_thermal_zones(void);
int check_charging_status(void);
int calculate_thermal_tier(int cpu_temp, int bat_temp);
void fix_battery_cycle_count(void);
int get_true_battery_cycles(void);
void track_charging_cycles(int is_charging);

#ifdef __cplusplus
}
#endif

#endif 
