$(warning KERNELRELEASE=$(KERNELRELEASE))
ifeq ($(KERNELRELEASE),)
ARCH := arm64

CROSS_COMPILE :=aarch64-linux-gnu-
KERNELDIR ?= /home/yangjunqiang/kernel/hikey/hikey-linaro

#KERNELDIR ?= /lib/modules/$(shell uname -r)/build  
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules 
	
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules*

.PHONY: all clean

else
    obj-m := cambricon_io-pgtable-arm.o

endif

