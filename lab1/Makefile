obj-m += ch_drv.o

all:
	@echo "Targets: clean, build, install"

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install: build
	sudo insmod ch_drv.ko
	sudo chmod 777 /dev/mychdev

remove: clean
	sudo rmmod ch_drv

test:
	sudo echo "10&6" > /dev/mychdev
	sudo cat /dev/mychdev
	sudo echo "5|6" > /dev/mychdev
	sudo cat /dev/mychdev
	sudo echo "5^6" > /dev/mychdev
	sudo cat /dev/mychdev