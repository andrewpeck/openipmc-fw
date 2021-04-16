.PHONY: CM7

.PHONY: clean build

CFLAGS = -mcpu=cortex-m7 -std=gnu11 -g3 -DDATA_IN_D2_SRAM \
				 -DUSE_HAL_DRIVER -DCORE_CM7 -DDEBUG -DSTM32H745xx -Os \
				 -ffunction-sections -fdata-sections -Wall -fstack-usage \
				 -MMD -MP --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb

INC = -I CM7/Core/Inc \
      -I CM7/Core/mcu_telnet_server \
			-I CM7/Core/printf \
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

#arm-none-eabi-gcc "../Core/Src/network_ctrls.c" -mcpu=cortex-m7 -std=gnu11 -g3 -DDATA_IN_D2_SRAM -DUSE_HAL_DRIVER -DCORE_CM7 -DDEBUG -DSTM32H745xx -c -I../Core/Inc -I../Core/mcu_telnet_server -I../Core/printf -I../Core/openipmc/src -I../Drivers/ksz8091 -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../LWIP/App -I../LWIP/Target -I../../Middlewares/Third_Party/LwIP/src/include -I../../Middlewares/Third_Party/LwIP/system -I../../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../../Middlewares/Third_Party/LwIP/src/include/lwip -I../../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../../Middlewares/Third_Party/LwIP/src/include/netif -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../../Middlewares/Third_Party/LwIP/system/arch -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/network_ctrls.d" -MT"Core/Src/network_ctrls.o" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "Core/Src/network_ctrls.o"


SRC_LWIP = Middlewares/Third_Party/LwIP/src/api/api_lib.c \
			  Middlewares/Third_Party/LwIP/src/api/api_msg.c \
			  Middlewares/Third_Party/LwIP/src/api/err.c \
			  Middlewares/Third_Party/LwIP/src/api/if_api.c \
			  Middlewares/Third_Party/LwIP/src/api/netbuf.c \
			  Middlewares/Third_Party/LwIP/src/api/netdb.c \
			  Middlewares/Third_Party/LwIP/src/api/netifapi.c \
			  Middlewares/Third_Party/LwIP/src/api/sockets.c \
			  Middlewares/Third_Party/LwIP/src/api/tcpip.c \
			  Middlewares/Third_Party/LwIP/src/apps/mqtt/mqtt.c \
			  Middlewares/Third_Party/LwIP/src/core/altcp.c \
			  Middlewares/Third_Party/LwIP/src/core/altcp_alloc.c \
			  Middlewares/Third_Party/LwIP/src/core/altcp_tcp.c \
			  Middlewares/Third_Party/LwIP/src/core/def.c \
			  Middlewares/Third_Party/LwIP/src/core/dns.c \
			  Middlewares/Third_Party/LwIP/src/core/inet_chksum.c \
			  Middlewares/Third_Party/LwIP/src/core/init.c \
			  Middlewares/Third_Party/LwIP/src/core/ip.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/autoip.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/dhcp.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/etharp.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/icmp.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/igmp.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/ip4.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_addr.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_frag.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/dhcp6.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/ethip6.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/icmp6.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/inet6.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/ip6.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/ip6_addr.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/ip6_frag.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/mld6.c \
			  Middlewares/Third_Party/LwIP/src/core/ipv6/nd6.c \
			  Middlewares/Third_Party/LwIP/src/core/mem.c \
			  Middlewares/Third_Party/LwIP/src/core/memp.c \
			  Middlewares/Third_Party/LwIP/src/core/netif.c \
			  Middlewares/Third_Party/LwIP/src/core/pbuf.c \
			  Middlewares/Third_Party/LwIP/src/core/raw.c \
			  Middlewares/Third_Party/LwIP/src/core/stats.c \
			  Middlewares/Third_Party/LwIP/src/core/sys.c \
			  Middlewares/Third_Party/LwIP/src/core/tcp.c \
			  Middlewares/Third_Party/LwIP/src/core/tcp_in.c \
			  Middlewares/Third_Party/LwIP/src/core/tcp_out.c \
			  Middlewares/Third_Party/LwIP/src/core/timeouts.c \
			  Middlewares/Third_Party/LwIP/src/core/udp.c \
			  Middlewares/Third_Party/LwIP/src/netif/bridgeif.c \
			  Middlewares/Third_Party/LwIP/src/netif/bridgeif_fdb.c \
			  Middlewares/Third_Party/LwIP/src/netif/ethernet.c \
			  Middlewares/Third_Party/LwIP/src/netif/lowpan6.c \
			  Middlewares/Third_Party/LwIP/src/netif/lowpan6_ble.c \
			  Middlewares/Third_Party/LwIP/src/netif/lowpan6_common.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/auth.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/ccp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/chap-md5.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/chap-new.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/chap_ms.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/demand.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/eap.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/ecp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/eui64.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/fsm.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/ipcp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/ipv6cp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/lcp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/magic.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/mppe.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/multilink.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/ppp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/pppapi.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/pppcrypt.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/pppoe.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/pppol2tp.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/pppos.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/upap.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/utils.c \
			  Middlewares/Third_Party/LwIP/src/netif/ppp/vj.c \
			  Middlewares/Third_Party/LwIP/src/netif/slipif.c \
			  Middlewares/Third_Party/LwIP/src/netif/zepif.c \
			  Middlewares/Third_Party/LwIP/system/OS/sys_arch.c

SRC_FREE_RTOS = Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
					 Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
					 Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
					 Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
					 Middlewares/Third_Party/FreeRTOS/Source/list.c \
					 Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
					 Middlewares/Third_Party/FreeRTOS/Source/queue.c \
					 Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
					 Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
					 Middlewares/Third_Party/FreeRTOS/Source/timers.c

SRC_HAL = Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
			 Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c

SRC_OPENIPMC = CM7/Core/openipmc/src/ipmb_0.c \
				CM7/Core/openipmc/src/ipmi_msg_command_switch.c \
				CM7/Core/openipmc/src/ipmi_msg_manager.c \
				CM7/Core/openipmc/src/ipmi_netfn_app.c \
				CM7/Core/openipmc/src/ipmi_netfn_picmg.c \
				CM7/Core/openipmc/src/ipmi_netfn_sensor_event.c \
				CM7/Core/openipmc/src/ipmi_netfn_storage.c \
				CM7/Core/openipmc/src/power_manager.c \
				CM7/Core/openipmc/src/sdr_manager.c \
				CM7/Core/openipmc/src/sensors_templates.c \
				CM7/Core/openipmc/examples/ipmc_custom_initialization.c \
				CM7/Core/openipmc/src/front_leds.c \
				CM7/Core/openipmc/src/fru_inventory_manager.c \
				CM7/Core/openipmc/src/fru_state_machine.c \
				CM7/Core/openipmc/src/handle_switch.c

SRC_APOLLO = CM7/Core/Src/apollo/apollo.c \
				 CM7/Core/Src/apollo/pim400.c

CFILES = $(SRC_LWIP) $(SRC_FREE_RTOS) $(SRC_HAL) $(SRC_OPENIPMC) $(SRC_APOLLO) \
				CM7/LWIP/Target/ethernetif.c \
				CM7/LWIP/App/lwip.c \
				CM7/Core/openipmc/src/device_id.c \
			  CM7/Drivers/ksz8091/ksz8091.c \
			  CM7/Core/printf/printf.c \
			  CM7/Core/Src/amc_gpios.c \
			  CM7/Core/Src/freertos.c \
			  CM7/Core/Src/main.c \
			  CM7/Core/Src/mgm_i2c.c \
			  CM7/Core/mcu_telnet_server/telnet_server.c \
				CM7/Core/Src/network_ctrls.c \
			  CM7/Core/Src/openipmc_brd_ctrls.c \
			  CM7/Core/Src/openipmc_hal.c \
			  CM7/Core/Src/openipmc_inits.c \
			  CM7/Core/Src/sense_i2c.c \
			  CM7/Core/Src/st_bootloader.c \
			  CM7/Core/Src/stm32h7xx_hal_msp.c \
			  CM7/Core/Src/stm32h7xx_hal_timebase_tim.c \
			  CM7/Core/Src/stm32h7xx_it.c \
			  CM7/Core/Src/syscalls.c \
			  CM7/Core/Src/sysmem.c \
			  Common/Src/system_stm32h7xx_dualcore_boot_cm4_cm7.c

COBJS = $(CFILES:.c=.o)

AOBJS = CM7/Core/Startup/startup_stm32h745xihx.o

all: build elf

build: $(COBJS) assembly

clean:
	rm -f $(COBJS) $(AOBJS)

assembly:
	arm-none-eabi-gcc -mcpu=cortex-m7 -g3 -c -x assembler-with-cpp -MMD -MP \
		-MF"CM7/Core/Startup/startup_stm32h745xihx.d" \
		-MT"CM7/Core/Startup/startup_stm32h745xihx.o" \
		--specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb \
		-o "CM7/Core/Startup/startup_stm32h745xihx.o" \
		"CM7/Core/Startup/startup_stm32h745xihx.s"

%.o: %.c
	arm-none-eabi-gcc $< $(CFLAGS) \
		-c $(INC) \
		-MF $(patsubst %.c,%.d,$<) \
		-MT "$@" \
		-o "$@"

elf:
	@arm-none-eabi-gcc -o "openipmc-fw_CM7.elf" $(COBJS) $(AOBJS) \
		-T "CM7/STM32H745XIHX_FLASH.ld" \
		-mcpu=cortex-m7 --specs=nosys.specs -Wl,-Map="openipmc-fw_CM7.map" \
		-Wl,--gc-sections -static --specs=nano.specs \
		-mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@arm-none-eabi-size   openipmc-fw_CM7.elf
	@arm-none-eabi-objdump -h -S  openipmc-fw_CM7.elf  > "openipmc-fw_CM7.list"
	@arm-none-eabi-objcopy  -O binary  openipmc-fw_CM7.elf  "openipmc-fw_CM7.bin"

load:
	dfu-util -s 0x08000000 -d 0483:df11 -a 0 -D ./openipmc-fw_CM7.bin

#CM7:
#/opt/st/stm32cubeide_1.5.1/headless-build.sh  -build CM7 -data $(PWD)
