/*********************************************************************************
 *      Copyright:  (C) 2023 CCNU
 *                  All rights reserved.
 *
 *       Filename:  lv_events.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2023年04月06日)
 *         Author:  Donald Shallwing <donald_shallwing@126.com>
 *      ChangeLog:  1, Release initial version on "2023年04月06日 10时03分27秒"
 *                 
 ********************************************************************************/

#include "lv_events.h"


#define BUZZER_PATH				"/dev/buzzer"
#define LED_PATH				"/dev/dtsled"
#define SHT20_PATH				"/dev/sht20"

void led(bool status){

	int							fd = -1;
	unsigned char 				databuf[1];

	fd = open(LED_PATH, O_RDWR);
	if(fd < 0){
		printf("Fail to open the LED device file! [%s]\n", strerror(errno));
		goto END;
	}

	if(status)
		databuf[0] = 1;
	else
		databuf[0] = 0;
	
	write(fd, databuf, sizeof(databuf));

END:
	close(fd);
	return ;
}

void buzzer(bool status){

	int							fd = -1;
	int							period = 0;
	char						buf[8] = {0};

	fd = open(BUZZER_PATH, O_RDWR);
	if(fd < 0){
		printf("Fail to open the buzzer device file! [%s]\n", strerror(errno));
		goto END;
	}

	if(status){
		strcpy(buf, "8");
		printf("On\n");
	}
	else{
		strcpy(buf, "0");
		printf("Off\n");
	}

	
	if(0 > write(fd, buf, sizeof(buf))){
		printf("Fail to activate the buzzer!\n [%s]\n", strerror(errno));
		printf("ENDDDDD!\n");
		goto END;
	}


END:
	close(fd);
	return ;
}


void sht20(double *temperature, double *humidity){

	int							fd = -1;
	unsigned char 				data[4] = {0};

	fd = open(SHT20_PATH, O_RDONLY);
	if(fd < 0){
		printf("Fail to open the device file '%s', [%s]\n", SHT20_PATH, strerror(errno));
		*temperature = 0.0;
		*humidity = 0.0;
		goto END;
	}

	if(0 > read(fd, data, sizeof(data))){
		printf("Fail to read the temperature! [%s]\n", strerror(errno));
		*temperature = 0.0;
		*humidity = 0.0;
		goto END;
	}
	else{
		*temperature = ((data[1] << 8) | data[0]) / 1000.0;
		*humidity = ((data[3] << 8) | data[2]) / 1000.0;
	}

END:
	close(fd);
	return ;
}