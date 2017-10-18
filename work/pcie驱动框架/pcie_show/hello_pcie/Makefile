$(warning KERNELRELEASE=$(KERNELRELEASE))
ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build  
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
	@sudo dmesg -c
	@clear
	@clear
	@sudo insmod ./hello.ko
	@dmesg
	@sudo rmmod hello

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules*

.PHONY: all clean

else
  obj-m := hello.o
endif

