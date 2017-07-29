//
// Created by Administrator on 2017/7/26.
//
#include "test.h"

void testf(void) {
    DL_ERR("this is testf() in libtest1.so");
    return;
}

void testg(void) {
    DL_ERR("this is testg() in libtest1.so after inject");
    return;
}
