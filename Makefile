.PHONY: clean

# 'make batch' will first update the submodules, and then run the full build.
batch: init hpm full_image

# Update all submodules, recursively
init:
	git submodule update --init --recursive

# Build rule for the CM4/CM7 FW + the bootloader.
# The resulting files are copied into project root.
hpm: hpm_firmware hpm_bootloader 

# Build rule for the CM4 and CM7 firmware
hpm_firmware:
	@cd openipmc-fw                                     && \
	 $(MAKE) hpm                                        && \
	 cp CM4/openipmc-fw_CM4.bin ../openipmc-fw_CM4.bin  && \
	 cp CM4/upgrade_CM4.hpm     ../upgrade_CM4.hpm      && \
	 cp CM7/openipmc-fw_CM7.bin ../openipmc-fw_CM7.bin  && \
	 cp CM7/upgrade_CM7.hpm     ../upgrade_CM7.hpm

# Build rule for the OpenIPMC bootloader
hpm_bootloader:
	@cd openipmc-fw-bootloader                                     && \
	 $(MAKE) hpm                                                   && \
	 cp CM7/openipmc-fw-bootloader_CM7.bin ../openipmc-fw_BL7.bin  && \
	 cp CM7/upgrade_BL7.hpm                ../upgrade_BL7.hpm

# Build rule to create the combined binary with CM4/CM7 firmware and the bootloader.
# Again, the combined binary will be copied into the project root.
full_image: hpm
	@cp openipmc-fw_CM4.bin openipmc-fw-full-image-builder/ && \
	 cp openipmc-fw_CM7.bin openipmc-fw-full-image-builder/ && \
	 cp openipmc-fw_BL7.bin openipmc-fw-full-image-builder/ && \
	 cd openipmc-fw-full-image-builder && \
	 $(MAKE) full_image && \
	 cp openipmc-fw_full.bin ../ && \
	 $(MAKE) full_hpm_image && \
	 cp upgrade_FULL.hpm ../

# Clean all build products from submodules, and also from the root directory of the project.
clean:
	@cd openipmc-fw && $(MAKE) clean
	@cd openipmc-fw-bootloader && $(MAKE) clean
	@cd openipmc-fw-full-image-builder && $(MAKE) clean
	@rm -f openipmc-fw-full-image-builder/openipmc-fw_CM7.bin
	@rm -f openipmc-fw-full-image-builder/openipmc-fw_CM4.bin
	@rm -f openipmc-fw-full-image-builder/openipmc-fw_BL7.bin
	@rm -f openipmc-fw_BL7.bin
	@rm -f openipmc-fw_CM4.bin
	@rm -f openipmc-fw_CM7.bin
	@rm -f openipmc-fw_full.bin
	@rm -f upgrade_BL7.hpm
	@rm -f upgrade_CM4.hpm
	@rm -f upgrade_CM7.hpm
	@rm -f upgrade_FULL.hpm
	@rm -f openipmc-fw_binaries.tar.gz

# Load firmware and option bytes via dfu-util, using a microUSB connection
dfu_load:
	@cd openipmc-fw-full-image-builder/ && \
	 $(MAKE) dfu_load

load_st:
	st-flash --freq 1000 --reset write ./openipmc-fw/CM7/openipmc-fw_CM7.bin 0x08000000

load_bootloader:
	st-flash --freq 1000 --reset write ./openipmc-fw-bootloader_CM7.bin 0x081E0000

gitlab:
	rm -rf builds
	gitlab-runner exec shell build-job
