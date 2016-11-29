//
// Created by Administrator on 2016/10/28.
//
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/auxvec.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/atomics.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <android/log.h>
#include <jni.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "linker_phdr.h"
#include "fcntl.h"
#include "linker.h"


#ifdef __cplusplus
extern "C" {
#endif


void test(const char *name) {
    //set_soinfo_pool_protection(PROT_READ | PROT_WRITE);
    soinfo* si = find_library(name);
    if (si != NULL) {
        si->CallConstructors();
    }
    //set_soinfo_pool_protection(PROT_READ);
}

JNIEXPORT jstring
JNICALL Java_zybornit_loadlib_LoadLibrary_loadlib(JNIEnv *env, jobject obj,
                                                  jstring j_str) {
    const char *c_str = NULL;
    char buff[128] = {0};
    jboolean isCopy;    // 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针
    c_str = env->GetStringUTFChars(j_str, &isCopy);
    if (c_str == NULL) {
        return NULL;
    }
    //int m = func_add(123, 456);
    //printf("C_str: %s \n", c_str);
    test(c_str);
    sprintf(buff, "hello %s ", c_str);
    env->ReleaseStringUTFChars(j_str, c_str);
    jstring a = env->NewStringUTF(buff);
    return a;

}


#ifdef __cplusplus
}
#endif

