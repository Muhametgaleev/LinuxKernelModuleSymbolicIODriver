obj-m += lab2.o

all:
	@echo "Targets: clean, build, install"

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install: build
	sudo insmod lab2.ko
	sudo chmod 777 /dev/vramdisk

remove: clean
	sudo rmmod lab2