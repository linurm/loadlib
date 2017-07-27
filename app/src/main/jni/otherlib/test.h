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
    __android_log_print(ANDROID_LOG_ERROR, "test", __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif
void testf();
void testg();
#ifdef __cplusplus
}
#endif

#endif //LOADLIB_TEST_H
