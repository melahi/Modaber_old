################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../VALfiles/parsing/DebugWriteController.cpp \
../VALfiles/parsing/fixyywrap.cpp \
../VALfiles/parsing/pddl+.cpp \
../VALfiles/parsing/ptree.cpp 

OBJS += \
./VALfiles/parsing/DebugWriteController.o \
./VALfiles/parsing/fixyywrap.o \
./VALfiles/parsing/pddl+.o \
./VALfiles/parsing/ptree.o 

CPP_DEPS += \
./VALfiles/parsing/DebugWriteController.d \
./VALfiles/parsing/fixyywrap.d \
./VALfiles/parsing/pddl+.d \
./VALfiles/parsing/ptree.d 


# Each subdirectory must supply rules for building sources it contributes
VALfiles/parsing/%.o: ../VALfiles/parsing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/sadra/masterThesis/Modaber" -I"/home/sadra/masterThesis/Modaber/VALfiles/parsing" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


