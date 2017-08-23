//
// Created by Administrator on 2017/8/12.
//
#include "hook_funciton.h"




test_fun old_test = NULL;
int halfHourFlag = 0;

int clock_gettime_hook(int clock_id, timespec *tp) {
    timespec t;
    int flag;
    flag = old_test(clock_id, &t);
    tp->tv_nsec = t.tv_nsec;
    tp->tv_sec = t.tv_sec + (halfHourFlag * 30 * 60/* - 5*/);

    return flag;
}