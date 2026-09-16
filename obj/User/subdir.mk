################################################################################
# MRS Version: 2.5.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32x035_it.c \
../User/gps.c \
../User/main.c \
../User/ssd1306.c \
../User/system_ch32x035.c 

C_DEPS += \
./User/ch32x035_it.d \
./User/gps.d \
./User/main.d \
./User/ssd1306.d \
./User/system_ch32x035.d 

OBJS += \
./User/ch32x035_it.o \
./User/gps.o \
./User/main.o \
./User/ssd1306.o \
./User/system_ch32x035.o 

DIR_OBJS += \
./User/*.o \

DIR_DEPS += \
./User/*.d \

DIR_EXPANDS += \
./User/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/alarm/mounriver-studio-projects/CH32X035/Tracker/Debug" -I"/home/alarm/mounriver-studio-projects/CH32X035/Tracker/Core" -I"/home/alarm/mounriver-studio-projects/CH32X035/Tracker/User" -I"/home/alarm/mounriver-studio-projects/CH32X035/Tracker/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

