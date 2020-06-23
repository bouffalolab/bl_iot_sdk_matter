
#CROSS_COMPILE=~/project/joylink/toolchain/hcc_riscv32/bin/riscv32-unknown-elf-
#CROSS_COMPILE=xtensa-esp32-elf-
#CROSS_COMPILE=~/project/broadlink/dnasystem_firmware/build/rtl8710/toolchain/gcc-arm-none-eabi-4_8-2014q3/bin/arm-none-eabi-
CROSS_COMPILE=~/project/broadlink/dnasystem_firmware/build/rda5981n/toolchain/gcc-arm-none-eabi-5_4-2016q3/bin/arm-none-eabi-
CROSS_COMPILE=~/project/joylink/toolchain/riscv64-unknown-elf-gcc-8.3.0-2019.08.0/bin/riscv64-unknown-elf-

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib

CFLAGS += -DuECC_PLATFORM=uECC_arch_other

#----------------------------------------------以下为开发者定制编译参数
#CFLAGS +=-Wshadow -Wpointer-arith -Waggregate-return -Winline  -Wunreachable-code -Wcast-align -Wredundant-decls 
#CFLAGS += -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -w -O2 -Wno-pointer-sign -fno-common -fmessage-length=0  -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-short-enums -DF_CPU=166000000L -std=gnu99
#CFLAGS += -mcpu=cortex-m4 -mthumb -Wall -ffunction-sections -fdata-sections -Os --specs=nano.specs -std=gnu99 -W
CFLAGS += -std=gnu99 -Og -gdwarf -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -fshort-enums -ffreestanding -fno-strict-aliasing -march=rv32imfc -mabi=ilp32f
