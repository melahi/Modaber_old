################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lingeling/lglib.c 

OBJS += \
./Lingeling/lglib.o 

C_DEPS += \
./Lingeling/lglib.d 


# Each subdirectory must supply rules for building sources it contributes
Lingeling/%.o: ../Lingeling/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


