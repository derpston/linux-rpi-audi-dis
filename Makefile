obj-m := dis.o

KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

.phony: dis
dis:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

