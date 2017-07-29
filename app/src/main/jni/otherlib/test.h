//
// Created by Administrator on 2017/7/26.
//

#ifndef __TEST_H__
#define __TEST_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>

#define DL_ERR(...) \
    __android_log_print(ANDROID_LOG_ERROR, "ZTAG", __VA_ARGS__)

#define DL_DEBUG(...) \
    __android_log_print(ANDROID_LOG_ERROR, "JTAG", __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void testf(void);

void testg(void);

#ifdef __cplusplus
}
#endif

#endif //LOADLIB_TEST_H
