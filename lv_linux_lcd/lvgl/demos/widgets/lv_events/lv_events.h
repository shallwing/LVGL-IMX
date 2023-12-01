/********************************************************************************
 *      Copyright:  (C) 2023 CCNU
 *                  All rights reserved.
 *
 *       Filename:  lv_events.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2023年04月06日)
 *         Author:  Donald Shallwing <donald_shallwing@126.com>
 *      ChangeLog:  1, Release initial version on "2023年04月06日 10时00分44秒"
 *                 
 ********************************************************************************/

#ifndef _LV_EVENTS_H
#define _LV_EVENTS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

extern void led(bool status);
extern void sht20(double *temperature, double *humidity);
extern void buzzer(bool status);

#endif
