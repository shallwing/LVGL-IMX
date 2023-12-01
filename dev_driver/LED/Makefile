LINUX_SRC=/home/$(shell whoami)/linux-imx/
PWD:=$(shell pwd)
EXTRA_CFLAGS+=-DMODULE
obj-m+=kernel_dtsled.o

modules:
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear

clear:
	@rm -f *.o *.cmd *.mod.c .*.cmd *.mod
	@rm -rf  *~ core .depend  .tmp_versions Module.symvers modules.order -f
	@rm -f .*ko.cmd .*.o.cmd .*.o.d

clean: clear
	@rm -f  *.ko
