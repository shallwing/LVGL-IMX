/*********************************************************************************
 *      Copyright:  (C) 2022 CCNU
 *                  All rights reserved.
 *
 *       Filename:  sht20.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2022年09月10日)
 *         Author:  Donald Shallwing <donald_shallwing@126.com>
 *      ChangeLog:  1, Release initial version on "2022年09月10日 23时23分01秒"
 *                 
 ********************************************************************************/

#include "sht20.h"

// ploy == x8 + x5 + x4 + x1
#define CRC8_POLY			0x131

int sht20_init(struct i2c_client *client){
	
	int								rv = -1;
	const char						cmd = SHT20_RST;

	rv = i2c_master_send(client, &cmd, sizeof(char));
	if(rv < 0){
		dev_err(&client->dev, "Fail to reset the sht20!");
		return -EINVAL;
	}
	
	mdelay(50);
	return 0;
}

int crc8_check(unsigned char *buffer, int len, int checksum){
	
	int								i, j;
	unsigned char					crc = 0x00;

	for(i=0; i<len; i++){
		crc ^= buffer[i];
		for(j=0; j<8; j++){
			if(crc & 0x80)
				crc = (crc << 1) ^ CRC8_POLY;
			else
				crc = (crc << 1);
		}
	}

	if(crc == checksum)
		return 1;
	else
		return 0;
}

int sht20_read_temperature(struct i2c_client *client, unsigned short *temperature){

	int								rv = -1;
	unsigned char					buffer[3] = {0};
	const char						cmd = SHT20_HOLD_T_MEASURE;

	rv = i2c_master_send(client, &cmd, sizeof(char));
	if(rv < 0){
		dev_err(&client->dev, "Fail to send the temperature measure command!");
		return -EINVAL;
	}
	
	rv = i2c_master_recv(client, buffer, sizeof(buffer));
	if(rv < 0){
		dev_err(&client->dev, "Fail to receive the data from SHT20!");
		return -EINVAL;
	}

	mdelay(85);

	if(!crc8_check(buffer, 2, buffer[2])){
		printk("CRC checkout error! Fail to get the temperature!\n");
		return -EINVAL;
	}
	
	*temperature = (buffer[0] << 8) | (buffer[1] & 0xFC);
	*temperature = ((*temperature * 175720) >> 16 ) - 46850;

	return 0;
}

int sht20_read_humidity(struct i2c_client *client, unsigned short *humidity){

	int								rv = -1;
	unsigned char					buffer[3] = {0};
	const char 						cmd = SHT20_HOLD_RH_MEASURE;

	rv = i2c_master_send(client, &cmd, sizeof(char));
	if(rv < 0){
		dev_err(&client->dev, "Fail to send the humidity measure command!");
		return -EINVAL;
	}

	mdelay(30);

	rv = i2c_master_recv(client, buffer, sizeof(buffer));
	if(rv < 0){
		dev_err(&client->dev, "Fail to receive the data from SHT20!");
		return -EINVAL;
	}
	
	if(!crc8_check(buffer, 2, buffer[2])){
		printk("CRC checkout error! Fail to get the humidity!\n");
		return -EINVAL;
	}
	
	*humidity = (buffer[0] << 8) | (buffer[1] & 0xFC);
	*humidity = ((*humidity * 125000) >> 16 ) - 6000;

	return 0;
}
