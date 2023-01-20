.PHONY: clean

hpm:
	make -C openipmc-fw/ hpm

clean:
	make -C openipmc-fw/ clean

load_usb:
	dfu-util -s 0x08000000 -d 0483:df11 -a 0 -D ./openipmc-fw/CM7/openipmc-fw_CM7.bin

load_st:
	st-flash --freq 1000 --reset write ./openipmc-fw/CM7/openipmc-fw_CM7.bin 0x08000000

load_bootloader:
	st-flash --freq 1000 --reset write ./openipmc-fw-bootloader_CM7.bin 0x081E0000

gitlab:
	rm -rf builds
	gitlab-runner exec shell build-job

init:
	git submodule update --init --recursive
