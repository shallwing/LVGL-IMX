#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEV_COUNT		1
#define DEV_NAME		"dtsled"
#define ON				0x01
#define OFF				0x00


struct dtsled_dev {
	dev_t				devid;
	int 				major, minor;
	int					led_gpio;
	struct cdev			cdev;
	struct device		*device;
	struct device_node	*device_node;
	struct class		*class;
};

struct dtsled_dev dtsled;

static ssize_t dtsled_write(struct file *filp, const char __user *buf, \
		size_t count, loff_t *ppos){
	int					rv = -1;
	unsigned char		databuf[1];
	struct dtsled_dev	*led = filp->private_data;

	rv = copy_from_user(databuf, buf, count);
	if(rv < 0)
		return -EINVAL;
	
	switch(databuf[0]){
		case(ON):
			gpio_set_value(led->led_gpio, ON);
			break;
		case(OFF):
			gpio_set_value(led->led_gpio, OFF);
			break;
		default:
			break;
	}
	return 0;
}

static int dtsled_open(struct inode *node, struct file *filp){

	filp->private_data = &dtsled;
	return 0;
}

static int dtsled_close(struct inode *node, struct file *filp){
	return 0;
}

static const struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = dtsled_write,
	.open = dtsled_open,
	.release = dtsled_close
};

static int __init dtsled_init(void){
			
	int			rv = -1;
	
// Allocate the device ID
	dtsled.major = 0;
	if(dtsled.major){
		dtsled.devid = MKDEV(dtsled.major, 0);
		register_chrdev_region(dtsled.devid, DEV_COUNT, DEV_NAME);
	}
	else{
		alloc_chrdev_region(&dtsled.devid, 0, DEV_COUNT, DEV_NAME);
		dtsled.major = MAJOR(dtsled.devid);
		dtsled.minor = MINOR(dtsled.devid);
	}

// Initialize cdev
	dtsled.cdev.owner = THIS_MODULE;
	cdev_init(&dtsled.cdev, &led_fops);

// Add structure body cdev
	cdev_add(&dtsled.cdev, dtsled.devid, DEV_COUNT);
		
// Create device node for modprobe .ko drive
	dtsled.class = class_create(THIS_MODULE, DEV_NAME);
	if(IS_ERR(dtsled.class)){
		return PTR_ERR(dtsled.class);
	}
// Create device node
	dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DEV_NAME);
	if(IS_ERR(dtsled.device)){
		return PTR_ERR(dtsled.device);
	}

// Locate the device info in dts file
	dtsled.device_node = of_find_node_by_path("/my_gpio_led/led0");
	if(NULL == dtsled.device_node){
		printk("Fail to find the device node in dts files!\n");
		return -EINVAL;
	}
// Read the GPIO info from the device node
 	dtsled.led_gpio = -1;
	dtsled.led_gpio = of_get_named_gpio(dtsled.device_node, "gpios", 0);
	if(dtsled.led_gpio < 0){
		printk("Can't find the gpio info!\n");
		return -EINVAL;
	}
	else{
		printk("The gpio number is %d\n", dtsled.led_gpio);
	}
// Request GPIO from system 
	rv = gpio_request(dtsled.led_gpio, "gpios");
	if(rv < 0){
		printk("Fail to request GPIO from system!\n");
		return -EINVAL;
	}
// Set the mode to input for terminal GPIO pin
	rv = -1;
	rv = gpio_direction_output(dtsled.led_gpio, OFF);
	if(rv < 0){
		printk("Fail to set the GPIO to input mode!\n");
		return -EINVAL;
	}

	return 0;
}


static void __exit dtsled_exit(void){

	gpio_set_value(dtsled.led_gpio, OFF);
	cdev_del(&dtsled.cdev);
	unregister_chrdev_region(dtsled.devid, DEV_COUNT);
	device_destroy(dtsled.class, dtsled.devid);
	class_destroy(dtsled.class);

	gpio_free(dtsled.led_gpio);
	return ;
}

module_init(dtsled_init);
module_exit(dtsled_exit);

MODULE_AUTHOR("Donald Shallwing");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This drive program is finished by Shallwing");
