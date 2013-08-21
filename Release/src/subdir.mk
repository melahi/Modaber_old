################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CVC4Problem.cpp \
../src/Modaber.cpp \
../src/MyAction.cpp \
../src/MyAtom.cpp \
../src/MyEnvironment.cpp \
../src/MyProblem.cpp \
../src/MyStateVariable.cpp \
../src/MyTimer.cpp \
../src/NumericalPlanningGraph.cpp \
../src/PreconditionFinder.cpp \
../src/SimpleModaber.cpp \
../src/SketchyPlan.cpp \
../src/Translator.cpp \
../src/Utilities.cpp \
../src/modaberMain.cpp 

OBJS += \
./src/CVC4Problem.o \
./src/Modaber.o \
./src/MyAction.o \
./src/MyAtom.o \
./src/MyEnvironment.o \
./src/MyProblem.o \
./src/MyStateVariable.o \
./src/MyTimer.o \
./src/NumericalPlanningGraph.o \
./src/PreconditionFinder.o \
./src/SimpleModaber.o \
./src/SketchyPlan.o \
./src/Translator.o \
./src/Utilities.o \
./src/modaberMain.o 

CPP_DEPS += \
./src/CVC4Problem.d \
./src/Modaber.d \
./src/MyAction.d \
./src/MyAtom.d \
./src/MyEnvironment.d \
./src/MyProblem.d \
./src/MyStateVariable.d \
./src/MyTimer.d \
./src/NumericalPlanningGraph.d \
./src/PreconditionFinder.d \
./src/SimpleModaber.d \
./src/SketchyPlan.d \
./src/Translator.d \
./src/Utilities.d \
./src/modaberMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


