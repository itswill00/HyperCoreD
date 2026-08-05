
#ifndef HYPERCORE_IPC_HPP
#define HYPERCORE_IPC_HPP

#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

int init_ipc_socket(void);
void close_ipc_socket(void);
void handle_ipc_events(int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif 
