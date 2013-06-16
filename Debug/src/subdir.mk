################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CVC4Problem.cpp \
../src/EvolutionaryModaber.cpp \
../src/NumericRPG.cpp \
../src/SketchyPlan.cpp \
../src/Translator.cpp \
../src/modaberMain.cpp 

OBJS += \
./src/CVC4Problem.o \
./src/EvolutionaryModaber.o \
./src/NumericRPG.o \
./src/SketchyPlan.o \
./src/Translator.o \
./src/modaberMain.o 

CPP_DEPS += \
./src/CVC4Problem.d \
./src/EvolutionaryModaber.d \
./src/NumericRPG.d \
./src/SketchyPlan.d \
./src/Translator.d \
./src/modaberMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/sadra/masterThesis/Modaber" -I"/home/sadra/masterThesis/Modaber/VALfiles/parsing" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


