################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Translation/Translator.cpp 

OBJS += \
./src/Translation/Translator.o 

CPP_DEPS += \
./src/Translation/Translator.d 


# Each subdirectory must supply rules for building sources it contributes
src/Translation/%.o: ../src/Translation/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


