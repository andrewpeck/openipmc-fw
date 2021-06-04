.PHONY: CM7

.PHONY: clean build elf

CFLAGS = -mcpu=cortex-m7 -std=gnu11 -g3 -DDATA_IN_D2_SRAM \
				 -DUSE_HAL_DRIVER -DCORE_CM7 -DDEBUG -DSTM32H745xx -Os \
				 -ffunction-sections -fdata-sections -Wall -fstack-usage \
				 -MMD -MP --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb

INC = -I CM7/Core/Src/ \
			-I CM7/Core/Src/apollo \
			-I CM7/Core/Inc \
			-I CM7/Core/mcu_telnet_server \
			-I CM7/Core/printf \
			-I CM7/Core/terminal \
			-I CM7/Core/openipmc/src \
			-I CM7/Drivers/ksz8091 \
			-I Drivers/STM32H7xx_HAL_Driver/Inc \
			-I Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
			-I Drivers/CMSIS/Device/ST/STM32H7xx/Include \
			-I Drivers/CMSIS/Include \
			-I Middlewares/Third_Party/FreeRTOS/Source/include \
			-I Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
			-I CM7/LWIP/App \
			-I CM7/LWIP/Target \
			-I Middlewares/Third_Party/LwIP/src/include \
			-I Middlewares/Third_Party/LwIP/system \
			-I Middlewares/Third_Party/LwIP/src/include/netif/ppp \
			-I Middlewares/Third_Party/LwIP/src/include/lwip \
			-I Middlewares/Third_Party/LwIP/src/include/lwip/apps \
			-I Middlewares/Third_Party/LwIP/src/include/lwip/priv \
			-I Middlewares/Third_Party/LwIP/src/include/lwip/prot \
			-I Middlewares/Third_Party/LwIP/src/include/netif \
			-I Middlewares/Third_Party/LwIP/src/include/compat/posix \
			-I Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa \
			-I Middlewares/Third_Party/LwIP/src/include/compat/posix/net \
			-I Middlewares/Third_Party/LwIP/src/include/compat/posix/sys \
			-I Middlewares/Third_Party/LwIP/src/include/compat/stdc \
			-I Middlewares/Third_Party/LwIP/system/arch \
			-I Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2

SRC_LWIP =	$(wildcard Middlewares/Third_Party/LwIP/src/api/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/src/apps/mqtt/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/src/core/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/src/core/ipv4/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/src/core/ipv6/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/src/netif/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/src/netif/ppp/*.c) \
						$(wildcard Middlewares/Third_Party/LwIP/system/OS/*.c )

SRC_FREE_RTOS = $(wildcard Middlewares/Third_Party/FreeRTOS/Source/*.c) \
								Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
								Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
								Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

SRC_HAL       = $(wildcard Drivers/STM32H7xx_HAL_Driver/Src/*.c)

SRC_OPENIPMC  = $(wildcard CM7/Core/openipmc/src/*.c)

SRC_APOLLO    = $(wildcard CM7/Core/Src/apollo/*.c)

SRC_TERMINAL  = $(wildcard CM7/Core/terminal/module/*.c) \
						 	  $(wildcard CM7/Core/terminal/lib/*.c) \
						 	  CM7/Core/terminal/terminal.c

CFILES = 	$(SRC_LWIP) $(SRC_FREE_RTOS) $(SRC_HAL) $(SRC_OPENIPMC) $(SRC_APOLLO) $(SRC_TERMINAL) \
					$(wildcard CM7/Core/Src/*.c) \
					CM7/Core/mcu_telnet_server/telnet_server.c \
					CM7/Core/printf/printf.c \
					CM7/Drivers/ksz8091/ksz8091.c \
				 	CM7/Core/openipmc/src/device_id.c \
				 	CM7/LWIP/App/lwip.c \
				 	CM7/LWIP/Target/ethernetif.c \
					Common/Src/system_stm32h7xx_dualcore_boot_cm4_cm7.c

COBJS = $(CFILES:.c=.o)

AOBJS = CM7/Core/Startup/startup_stm32h745xihx.o

all: elf
#elf

headers:
	@echo "Generating headers"
	@cd CM7/Core && sh Src/header_gen.sh && cd - > /dev/null

build: $(COBJS) $(AOBJS) headers

clean:
	rm -f $(COBJS) $(CFILES:.c=.su) $(CFILES:.c=.d) $(AOBJS)
	rm openipmc-fw_CM7.bin

%.o: %.s
	@echo "Building $<"
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -c -x assembler-with-cpp -MMD -MP \
		-MF$(patsubst %.s,%.d,$<) \
		-MT$(patsubst %.s,%.o,$<) \
		--specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb \
		-o "$@" "$<"

%.o: %.c
	@echo "Building $<"
	@arm-none-eabi-gcc $< $(CFLAGS) \
		-c $(INC) \
		-MF $(patsubst %.c,%.d,$<) \
		-MT "$@" \
		-o "$@"

elf: build
	echo $(COBJS)
	@echo "Building final binary"
	@arm-none-eabi-gcc -o "openipmc-fw_CM7.elf" $(COBJS) $(AOBJS) \
		-T "CM7/STM32H745XIHX_FLASH.ld" \
		-mcpu=cortex-m7 --specs=nosys.specs -Wl,-Map="openipmc-fw_CM7.map" \
		-Wl,--gc-sections -static --specs=nano.specs \
		-mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@arm-none-eabi-size   openipmc-fw_CM7.elf
	@arm-none-eabi-objdump -h -S  openipmc-fw_CM7.elf  > "openipmc-fw_CM7.list"
	@arm-none-eabi-objcopy  -O binary  openipmc-fw_CM7.elf  "openipmc-fw_CM7.bin"

load_usb:
	dfu-util -s 0x08000000 -d 0483:df11 -a 0 -D ./openipmc-fw_CM7.bin

load_st:
	st-flash --reset write ./openipmc-fw_CM7.bin 0x08000000

gitlab:
	gitlab-runner exec shell build-job

#CM7:
#/opt/st/stm32cubeide_1.5.1/headless-build.sh  -build CM7 -data $(PWD)
