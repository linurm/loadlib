#pragma once

#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include "../otherlib/test.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define printWordHex(addr) \
    DL_DEBUG(" 0x%x", (int) addr);\
    DL_DEBUG(" 0x%x", *addr);\
    DL_DEBUG("   %x %x %x %x", *(uint8_t *) addr,*((uint8_t *)addr+1),*((uint8_t *) addr+2),*((uint8_t *) addr+3))


#define printHalfHex(addr) \
    DL_DEBUG(" 0x%x", (int) addr)\
    DL_DEBUG("   0x%x", *(uint16_t *) addr)


#define printByteHex(addr) \
    DL_DEBUG(" 0x%x", (int) addr)\
    DL_DEBUG("   %x", *(char *) addr)


#define printHex(addr) \
    DL_DEBUG(" 0x%x", (int) addr)


extern int injectLib(void);
int inject_remote_process(pid_t target_pid, const char *library_path, const char *function_name,
                          void *param, size_t param_size);

int find_pid_of(const char *process_name);

void *get_module_base(pid_t pid, const char *module_name);

#ifdef __cplusplus
}
#endif


struct inject_param_t {
    pid_t from_pid;
};
