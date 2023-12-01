/********************************************************************************
 *      Copyright:  (C) 2022 CCNU
 *                  All rights reserved.
 *
 *       Filename:  sht20.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2022年09月10日)
 *         Author:  Donald Shallwing <donald_shallwing@126.com>
 *      ChangeLog:  1, Release initial version on "2022年09月10日 23时23分31秒"
 *                 
 ********************************************************************************/

#ifndef _SHT20_H
#define _SHT20_H

#include <linux/i2c.h>
#include <linux/delay.h>

#define SHT20_ADDR					0x40
#define SHT20_RST					0xFE
#define SHT20_HOLD_T_MEASURE		0xE3		
#define SHT20_HOLD_RH_MEASURE		0xE5
#define SHT20_NOHOLD_T_MEASURE		0xF3
#define SHT20_NOHOLD_RH_MEASURE		0xF5

extern int sht20_i2c_init(struct i2c_client *client);
extern int sht20_read_temperature(struct i2c_client *client, unsigned short *temperature);
extern int sht20_read_humidity(struct i2c_client *client, unsigned short *humidity);
extern int crc8_check(unsigned char *buffer, int len, int checksum);

#endif
