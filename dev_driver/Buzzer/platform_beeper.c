/*********************************************************************************
 *      Copyright:  (C) 2022 CCNU
 *                  All rights reserved.
 *
 *       Filename:  platform_buzzer.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2022年12月29日)
 *         Author:  Donald Shallwing <donald_shallwing@126.com>
 *      ChangeLog:  1, Release initial version on "2022年12月29日 15时49分13秒"
 *                 
 ********************************************************************************/

#include <linux/kstrtox.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>


#define DEVNAME							"buzzer"
#define DEVCOUNT						1
#define DEVPATH							"/pwm-buzzer"

static int								major = 0;
struct buzzer_dev {
	char								name[16];
	int									pwm_id;
	struct pwm_device					*pwm_dev;
};

struct buzzer_priv { 
	struct cdev 						cdev;
	struct class						*class;
	struct buzzer_dev					buzzer[];
};


static const struct of_device_id buzzer_match_table[] = {
	{.compatible = "pwm-beeper"},
	{},
};

static int buzzer_open(struct inode *node, struct file *file){
	struct buzzer_priv					*buzzer = NULL;
	buzzer = container_of(node->i_cdev, struct buzzer_priv, cdev);
	file->private_data = buzzer;	
	return 0;
}

static ssize_t buzzer_show(struct device *dev, struct device_attribute *attr, \
		char *buf){

	struct buzzer_priv					*priv = dev_get_drvdata(dev);
	long 								period = pwm_get_period(priv->buzzer[0].pwm_dev);

	return sprintf(buf, "%ld", period); 
}

static ssize_t buzzer_store(struct device *dev, struct device_attribute *attr, \
		const char *buf, size_t count){
	
	struct buzzer_priv					*priv = dev_get_drvdata(dev);
	long long							period = 0;
	int									rv = -1;

	rv = kstrtoull(buf, 0, &period);
	if(rv >= 0){
		if(period){
			period *= 10000;
			pwm_config(priv->buzzer[0].pwm_dev, period / 2, period);
			pwm_enable(priv->buzzer[0].pwm_dev);
		}else{
			pwm_config(priv->buzzer[0].pwm_dev, 0, 0);
			pwm_disable(priv->buzzer[0].pwm_dev);
		}
	}
	
	return count;
}


static DEVICE_ATTR(beepdev, 0644, buzzer_show, buzzer_store);


static ssize_t buzzer_write(struct file *filp, const char __user *buf, \
		size_t count, loff_t *offset){

	unsigned long long					frq = 0;
	int									rv = -1;
	struct buzzer_priv					*priv = filp->private_data;
	char								databuf[16];

	if(!priv){
		printk("The priv is NULL!\n");
		return -EINVAL;
	}

	rv = copy_from_user(databuf, buf, count);		
	if(rv < 0){
		printk("Fail to write the user data to kernel!\n");
		return -EINVAL;
	}

	if(kstrtoull(databuf, 0, &frq) < 0){
		printk("Fail to convert string to unsigned long long number!\n");
		return -EINVAL;
	}

	if(frq == 0)
		pwm_disable(priv->buzzer[0].pwm_dev);
	else{
		frq *= 10000;
		pwm_config(priv->buzzer[0].pwm_dev, frq / 2, frq);
		pwm_enable(priv->buzzer[0].pwm_dev);
	}

	return 0;
}

static int buzzer_close(struct inode *node, struct file *file){
	
	return 0;
}

static int sizeof_buzzer(int leds){
	return sizeof(struct buzzer_priv) + leds * sizeof(struct buzzer_dev);
}

struct file_operations buzzer_ops = {
	.owner = THIS_MODULE,
	.open = buzzer_open,
	.release = buzzer_close,
	.write = buzzer_write
};

static int pwm_init(struct platform_device *priv){
	
	int									rv = -1;
	struct device 						*dev = &priv->dev;
	struct device_node					*buzzer_np = NULL;
	struct buzzer_priv					*buzzer_priv = NULL;
	uint32_t							pwm_prop[3] = {0}; 
	uint32_t							duty = 0, period = 0;

	buzzer_priv = devm_kzalloc(dev, sizeof_buzzer(DEVCOUNT), GFP_KERNEL);
	if(IS_ERR(buzzer_priv)){
		printk("Fail to allocate the property structure to buzzer!\n");
		return -ENOMEM;
	}

	buzzer_np = of_find_node_by_path(DEVPATH);
	if(IS_ERR(buzzer_np)){
		printk("Fail to find the buzzer device node!\n");
		return -EINVAL;
	}

	rv = of_property_read_u32_array(buzzer_np, "pwms", pwm_prop, 3);
	if(rv < 0){
		printk("Fail to read the pwms property!\n");
		return -EINVAL;
	}
	duty = pwm_prop[1];
	period = pwm_prop[2];

	buzzer_priv->buzzer[0].pwm_dev = devm_of_pwm_get(&priv->dev, buzzer_np, NULL);
	if(IS_ERR(buzzer_priv->buzzer[0].pwm_dev)){
		printk("Fail to request the pwm device!\n");
		return -EINVAL;
	}

	rv = pwm_config(buzzer_priv->buzzer[0].pwm_dev, duty, period);
	if(rv < 0){
		printk("Fail to config the pwm pin!\n");
		return -EINVAL;
	}
	pwm_enable(buzzer_priv->buzzer[0].pwm_dev);


	platform_set_drvdata(priv, buzzer_priv);
	return 0;
}

static int buzzer_probe(struct platform_device *priv){
	
	struct buzzer_priv					*buzzer;
	struct device						*device;
	dev_t								devid;
	int									rv = -1;

	if(pwm_init(priv) < 0)
		return -EINVAL;
	else
		buzzer = platform_get_drvdata(priv);
	
	if(!buzzer){
		printk(KERN_ERR "Fail to get the driver's data!\n");
		return -EINVAL;
	}

	if(0 != major){
		devid = MKDEV(major, 0);
		register_chrdev_region(devid, DEVCOUNT, DEVNAME);
	}
	else{
		alloc_chrdev_region(&devid, 0, DEVCOUNT, DEVNAME);
		major = MAJOR(devid);
	}

	cdev_init(&buzzer->cdev, &buzzer_ops);
	buzzer->cdev.owner = THIS_MODULE;
	if(IS_ERR(&buzzer->cdev)){
		printk("Can't allocate a new cdev to PWM device for %s!\n", DEVNAME);
		return -EINVAL;
	}
	cdev_add(&buzzer->cdev, devid, DEVCOUNT);

	buzzer->class = class_create(THIS_MODULE, DEVNAME);
	if(IS_ERR(buzzer->class)){
		printk("Can't allocate a new class for PWM device for %s!\n", DEVNAME);
		return -EINVAL;
	}

	device = device_create(buzzer->class, NULL, devid, NULL, DEVNAME);
	if(IS_ERR(device)){
		printk("Can't allocate a 'device' to %s!\n", DEVNAME);
		return -EINVAL;
	}
	
	rv = device_create_file(device, &dev_attr_beepdev);
	if(0 > rv){
		printk("Fail to create /sys/class file!\n");
		return -EINVAL;
	}
	else
		dev_set_drvdata(device, buzzer);

	platform_set_drvdata(priv, buzzer);
	return 0;
}

static int buzzer_remove(struct platform_device *priv){

	struct buzzer_priv			*buzzer = platform_get_drvdata(priv);
	int							i = 0;
	dev_t						devid;

	for(i=0; i<DEVCOUNT; i++){
		devid = MKDEV(major, i);
		device_destroy(buzzer->class, devid);
	}

	class_destroy(buzzer->class);
	cdev_del(&buzzer->cdev);
	unregister_chrdev_region(MKDEV(major, 0), DEVCOUNT);

	pwm_disable(buzzer->buzzer[0].pwm_dev);
	return 0;
}

static struct platform_driver buzzer_driver = {
	.probe = buzzer_probe,
	.remove = buzzer_remove,
	.driver = {
		.name = DEVNAME,
		.of_match_table = buzzer_match_table,
	},
};

static int __init buzzer_init(void){

	int							rv = -1;
	rv = platform_driver_register(&buzzer_driver);
	if(rv < 0){
		printk(KERN_ERR "%s:%d Can't registe a buzzer driver!\n", __FUNCTION__, \
				__LINE__);
		return rv;
	}
	return 0;
}

static void __exit buzzer_exit(void){

	printk("The beeper has been removed!\n");
	platform_driver_unregister(&buzzer_driver);
	return ;
}

module_init(buzzer_init);
module_exit(buzzer_exit);

MODULE_DEVICE_TABLE(of, buzzer_match_table);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is used for PWM pins test");
MODULE_AUTHOR("Donald Shallwing");
