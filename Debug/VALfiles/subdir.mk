################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../VALfiles/Environment.cpp \
../VALfiles/FastEnvironment.cpp \
../VALfiles/FuncAnalysis.cpp \
../VALfiles/SimpleEval.cpp \
../VALfiles/TIM.cpp \
../VALfiles/TimSupport.cpp \
../VALfiles/TypeStripWC.cpp \
../VALfiles/TypedAnalyser.cpp \
../VALfiles/instantiation.cpp \
../VALfiles/typecheck.cpp 

OBJS += \
./VALfiles/Environment.o \
./VALfiles/FastEnvironment.o \
./VALfiles/FuncAnalysis.o \
./VALfiles/SimpleEval.o \
./VALfiles/TIM.o \
./VALfiles/TimSupport.o \
./VALfiles/TypeStripWC.o \
./VALfiles/TypedAnalyser.o \
./VALfiles/instantiation.o \
./VALfiles/typecheck.o 

CPP_DEPS += \
./VALfiles/Environment.d \
./VALfiles/FastEnvironment.d \
./VALfiles/FuncAnalysis.d \
./VALfiles/SimpleEval.d \
./VALfiles/TIM.d \
./VALfiles/TimSupport.d \
./VALfiles/TypeStripWC.d \
./VALfiles/TypedAnalyser.d \
./VALfiles/instantiation.d \
./VALfiles/typecheck.d 


# Each subdirectory must supply rules for building sources it contributes
VALfiles/%.o: ../VALfiles/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I../VALfiles/parsing -I.././ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


