.PHONY: clean

hpm:
	make -C openipmc-fw/CM7/ hpm

headers:
	@echo "Generating headers"
	@cd CM7/Core && sh Src/header_gen.sh && cd - > /dev/null
	touch CM7/Core/Src/terminal_impl.c
	make CM7/Core/Src/terminal_impl.o
	touch CM7/Core/Src/openipmc_inits.c
	make CM7/Core/Src/openipmc_inits.o

clean:
	make -C openipmc-fw/CM7/ clean

generate_upgrade_file: generate_upgrade_file.c
	gcc generate_upgrade_file.c -o generate_upgrade_file  -lcrypto -lssl -lz

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
