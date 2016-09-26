
# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
#KERNEL_SRC = /usr/src/kernels/2.6.18-1.2257.fc5.mpls.1.955-i686/
EXTRA_CFLAGS += -I/usr/realtime/include -ffast-math -mhard-float 
# \-I/usr/src/kernels/2.6.18-1.2257.fc5.mpls.1.955-i686/include/linux

ifeq ($(KERNELRELEASE),)   
    # Assume the source tree is where the running kernel was built
    # You should set KERNELDIR in the environment if it's elsewhere
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    # The current directory is passed to sub-makes as argument
    PWD := $(shell pwd)
	INCLPTH := /usr/realtime/include/    

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else
	# called from kernel build system: just declare what our modules are
	obj-m := q8Driv.o #Q4Init.o #ModTest.o
	q8Driv-objs := q8Driv-patched.o q8Driv-betti.o
endif


