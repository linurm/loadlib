//
// Created by Administrator on 2016/10/28.
//
#include <stdio.h>
#include <jni.h>
#include "linker.h"


#ifdef __cplusplus
extern "C" {
#endif


void test(const char *name) {
    //set_soinfo_pool_protection(PROT_READ | PROT_WRITE);
    soinfo *si = find_library(name);
    if (si != NULL) {
        si->CallConstructors();
    }
    //set_soinfo_pool_protection(PROT_READ);
}

jmethodID get_JavaFunction(JNIEnv *env, jobject obj) {
    jclass native_clazz = env->GetObjectClass(obj);
    jmethodID fieldID_sm = env->GetMethodID(native_clazz, "setMessage", "Ljava/lang/String;");
    env->CallIntMethod(obj, fieldID_sm, "");
    return fieldID_sm;
}
JNIEnv *g_env;
jobject g_obj;
jmethodID getMessage;

void setTextView(unsigned char *a) {
    jstring jstrName = g_env->NewStringUTF((char *)a);
    g_env->CallIntMethod(g_obj, getMessage, jstrName);
}

JNIEXPORT
jstring
JNICALL Java_zybornit_loadlib_LoadLibrary_loadlib(JNIEnv *env, jobject obj,
                                                  jstring j_str) {
    const char *c_str = NULL;
    char buff[128] = {0};
    jboolean isCopy;    // 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针
    g_env = env;
    g_obj = obj;

    getMessage = get_JavaFunction(env, obj);

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

