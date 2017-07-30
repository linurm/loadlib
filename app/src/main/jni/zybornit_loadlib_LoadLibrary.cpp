//
// Created by Administrator on 2016/10/28.
//
#include <stdio.h>
#include <jni.h>
#include "linker_phdr.h"
#include "linker.h"
#include "inject/inject.h"
//#include <android_runtime/AndroidRuntime.h>


#ifdef __cplusplus
extern "C" {
#endif


void loadLibTest(const char *name) {
    void *handle;
    void *vonLoad;

    testf();
#if 1
    //set_soinfo_pool_protection(PROT_READ | PROT_WRITE);
    soinfo *si = find_library(name);
    if (si != NULL) {
        si->CallConstructors();
    }
    //set_soinfo_pool_protection(PROT_READ);
#else
#define RTLD_LAZY 1
    handle = do_zlopen(name, RTLD_LAZY);

    vonLoad = zlsym(handle, "JNI_OnLoad");

    do_zlclose((soinfo *) handle);

#endif
}
JavaVM *g_jvm = NULL;
//JNIEnv *g_env;
jobject g_obj;
jobject g_ma_obj;
jmethodID g_sendMessage;

//void get_JavaFunction() {
//    //jclass cls = env->FindClass("zybornit/loadlib/DisplayHandler");
//
////调用父类中的function方法(但是会执行子类的方法)
//    //env->
//    //jstring ms =
//    JNIEnv *mJNIEnv;
//    g_jvm->GetEnv((void **) &mJNIEnv, JNI_VERSION_1_4);
//    //mJNIEnv->CallVoidMethod(g_ma_obj, g_sendMessage, 100);
//
//    //g_env->CallVoidMethod(g_ma_obj, id_sendMessage, 100, 10);
////调用父类中的function方法(执行就是父类中的function方法)
//    //env->CallNonvirtualVoidMethod(father, clazz_father, id_father_function);
//
//}


void setTextView(unsigned char *a) {
    JNIEnv *mJNIEnv;
    g_jvm->GetEnv((void **) &mJNIEnv, JNI_VERSION_1_4);
    jstring jstrName = mJNIEnv->NewStringUTF((char *) a);
    mJNIEnv->CallVoidMethod(g_ma_obj, g_sendMessage, jstrName, 10);
    //get_JavaFunction();
    mJNIEnv->DeleteLocalRef(jstrName);
}

//static void attach_thread_vm() {
//    JavaVMAttachArgs args;
//    char name[] = "load lib vm Thread";
//    args.version = JNI_VERSION_1_6;
//    args.name = name;
//    args.group = NULL;
////    JavaVM* vm = AndroidRuntime::getJavaVM();
////    vm->AttachCurrentThread(&g_env, &args);
//}
//static void dettach_thread_vm() {
////    JavaVM* vm = AndroidRuntime::getJavaVM();
////    vm->DetachCurrentThread();
//}
JNIEXPORT
jstring
JNICALL Java_zybornit_loadlib_LoadLibrary_loadlib(JNIEnv *env, jobject obj,
                                                  jstring j_str) {
    const char *c_str = NULL;
    char buff[128] = {0};
    jboolean isCopy;    // 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针
    //g_env = env;
    //g_obj = obj;



    c_str = env->GetStringUTFChars(j_str, &isCopy);
    if (c_str == NULL) {
        return NULL;
    }
    //int m = func_add(123, 456);
    //printf("C_str: %s \n", c_str);
    //DL_ERR("C_str: %s ", c_str);
    //DL_ERR("C_str");
    injectLib();

    loadLibTest(c_str);
    sprintf(buff, "hello %s ", c_str);
    env->ReleaseStringUTFChars(j_str, c_str);
    jstring a = env->NewStringUTF(buff);

    return a;

}
JNIEXPORT
void
JNICALL Java_zybornit_loadlib_LoadLibrary_setJNIEnv(JNIEnv *env, jobject obj) {
    (env)->GetJavaVM(&g_jvm);
    g_obj = (env)->NewGlobalRef(obj);
    jclass loadlib_clazz = env->GetObjectClass(obj);//LoadLibrary
    jfieldID mainActivity_id = env->GetFieldID(loadlib_clazz, "mainActivity",
                                               "Lzybornit/loadlib/MainActivity;");
    jobject m_obj = env->GetObjectField(obj, mainActivity_id);
    g_ma_obj = (env)->NewGlobalRef(m_obj);
    jclass clazz_MainActivity = env->FindClass("zybornit/loadlib/MainActivity");
    g_sendMessage = env->GetMethodID(clazz_MainActivity, "sendMessageToTextView",
                                     "(Ljava/lang/String;I)V");
    //get_JavaFunction();
}

JNIEXPORT
void
JNICALL Java_zybornit_loadlib_LoadLibrary_releaseJNIEnv(JNIEnv *env, jobject obj) {
    env->DeleteGlobalRef(g_ma_obj);
    env->DeleteGlobalRef(g_obj);
}

#ifdef __cplusplus
}
#endif

