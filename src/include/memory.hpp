
#ifndef HYPERCORE_MEMORY_HPP
#define HYPERCORE_MEMORY_HPP

#include "common.hpp"
#include "sysfs.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void apply_memory_tuning(void);
void tune_memory_pressure(void);

#ifdef __cplusplus
}
#endif

#endif 
