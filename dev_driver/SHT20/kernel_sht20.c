/*********************************************************************************
 *      Copyright:  (C) 2022 CCNU
 *                  All rights reserved.
 *
 *       Filename:  kernel_sht20.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2022年09月02日)
 *         Author:  Donald Shallwing <donald_shallwing@126.com>
 *      ChangeLog:  1, Release initial version on "2022年09月02日 14时58分03秒"
 *                 
 ********************************************************************************/

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include "sht20.h"

#define DEVNAME				"sht20"
#define DEVCOUNT			1

struct sht20_dev {
	dev_t					devid;
	int 					major, minor;
	struct cdev				cdev;
	struct class			*class;
	struct device			*device;
	struct i2c_client		*client;
};
struct sht20_dev sht20;

static const struct of_device_id sht20_match_table[] = {
	{.compatible = "DS,sht20"},
	{}
};

static ssize_t sht20_show(struct device *dev, struct device_attribute *attr, \
		char *buf){
	int						rv = 0;
	unsigned short 			temperature = 0, humidity = 0;
	int						data[4] = {0};

	rv = sht20_read_temperature(sht20.client, &temperature);
	if(rv < 0)
		return rv;

	rv = sht20_read_humidity(sht20.client, &humidity);
	if(rv < 0)
		return rv;
	
	data[0] = (0xFF & temperature);
	data[1] = (temperature >> 8) & 0xFF;
	data[2] = (0xFF & humidity);
	data[3] = (humidity >> 8) & 0xFF;
	
	return sprintf(buf, "%d %d %d %d", data[0], data[1], data[2], data[3]);
}

static ssize_t sht20_store(struct device *dev, struct device_attribute *attr, \
		const char *buf, size_t count){
	return count;
}

static DEVICE_ATTR(shtdev, 0644, sht20_show, sht20_store);

static int sht20_open(struct inode *node, struct file *filp){
	filp->private_data = &sht20;
	return 0;
}

static int sht20_close(struct inode *node, struct file *filp){
	return 0;
}

static ssize_t sht20_read(struct file *filp, __user char *buf, \
		size_t count, loff_t *ppos){
	
	unsigned char			data[4] = {0};
	int						rv = 0;
	unsigned short			temperature = 0, humidity = 0;
	
	rv = sht20_read_temperature(sht20.client, &temperature);
	if(rv < 0)
		return rv;
	rv = sht20_read_humidity(sht20.client, &humidity);
	if(rv < 0)
		return rv;
	
	/* Print the value of REG for debuging */
//	printk("temperature = %d, humidity = %d\n", temperature, humidity);
	
	data[0] = (0xFF & temperature);
	data[1] = (temperature >> 8) & 0xFF;
	data[2] = (0xFF & humidity);
	data[3] = (humidity >> 8) & 0xFF;

	rv = copy_to_user(buf, data, sizeof(data));
	return rv;
}

static struct file_operations sht20_fops = {
	.owner = THIS_MODULE,
	.open = sht20_open,
	.read = sht20_read,
	.release = sht20_close
};


static int sht20_init(struct i2c_client *client, const struct i2c_device_id *id){

	int						rv = 0;

	if(sht20.major){
		sht20.devid = MKDEV(sht20.major, 0);
		register_chrdev_region(sht20.devid, DEVCOUNT, DEVNAME);
	}
	else{
		alloc_chrdev_region(&sht20.devid, 0, DEVCOUNT, DEVNAME);
		sht20.major = MAJOR(sht20.devid);
		sht20.minor = MINOR(sht20.devid);
	}

	sht20.cdev.owner = THIS_MODULE;
	cdev_init(&sht20.cdev, &sht20_fops);
	cdev_add(&sht20.cdev, sht20.devid, DEVCOUNT);
	if(IS_ERR(&sht20.cdev)){
		printk("Fail to add a cdev!\n");
		return PTR_ERR(&sht20.cdev);
	}

	sht20.class = class_create(THIS_MODULE, DEVNAME);
	if(IS_ERR(sht20.class)){
		printk("Fail to create the device class!\n");
		return PTR_ERR(sht20.class);
	}

	sht20.device = device_create(sht20.class, NULL, sht20.devid, NULL, DEVNAME);
	if(IS_ERR(sht20.device)){
		printk("Fail to create the device node!\n");
		return PTR_ERR(sht20.device);
	}

	sht20.client = client;

	rv = device_create_file(sht20.device, &dev_attr_shtdev);
	if(rv < 0){
		printk("Fail to create sht20 sysclass file in /sys/class!\n");
		return rv;
	}
	dev_set_drvdata(sht20.device, &sht20);

	return rv;
}

static int sht20_exit(struct i2c_client *client){

	unregister_chrdev(sht20.major, DEVNAME);
	cdev_del(&sht20.cdev);

	device_destroy(sht20.class, sht20.devid);
	class_destroy(sht20.class);

	return 0;
}

static struct i2c_driver sht20_driver = {
	.probe = sht20_init,
	.remove = sht20_exit,
	.driver = {
		.owner = THIS_MODULE,
		.name = DEVNAME,
		.of_match_table = sht20_match_table,
	},
};

static __init int sht20_probe(void){
	
	int						rv = 0;

// Using i2c_add_driver to load i2c drivers
	rv = i2c_add_driver(&sht20_driver);
	if(rv < 0){
		printk(KERN_ERR "%s:%d: Can't register platform driver!\n", __FUNCTION__, \
				__LINE__);
		return rv;
	}

	return 0;
}

static __exit void sht20_remove(void){

// Using i2c_del_driver to remove i2c drivers
	printk("The sht20 module has been removed!\n");
	i2c_del_driver(&sht20_driver);
	return ;
}

module_init(sht20_probe);
module_exit(sht20_remove);

MODULE_DEVICE_TABLE(of, sht20_match_table);
MODULE_AUTHOR("Donald Shallwing");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("This module is finished by Donald Shallwing");
