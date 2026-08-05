
#ifndef HYPERCORE_SYSFS_HPP
#define HYPERCORE_SYSFS_HPP

#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void sysfs_write(const char *path, const char *val);
void sysfs_write_fallback(const char *paths[], const char *val);
int sysfs_read_int(const char *path);
int sysfs_read_str(const char *path, char *out_buf, size_t max_len);
void save_baseline_nodes(void);
void restore_baseline_nodes(void);
void update_module_prop_status(const char *status);

#ifdef __cplusplus
}
#endif

#endif 
