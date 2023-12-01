CC=/opt/buildroot/cortexA7/bin/arm-buildroot-linux-gnueabihf-gcc
LINUX_SRC=/home/$(shell whoami)/linux-imx/
APP+=sht20_sysfs_test

PWD:=$(shell pwd)

EXTRA_CFLAGS+=-DMODULE

obj-m+=Ksht20.o
Ksht20-objs=sht20.o kernel_sht20.o

modules:
	@$(CC) $(APP).c -o $(APP) -g
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear

clear:
	@rm -f *.o *.cmd *.mod.c .*.cmd *.mod
	@rm -rf  *~ core .depend  .tmp_versions Module.symvers modules.order -f
	@rm -f .*ko.cmd .*.o.cmd .*.o.d

clean: clear
	@rm -f  *.ko
