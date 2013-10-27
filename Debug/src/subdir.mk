################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Agent.cpp \
../src/CVC4Problem.cpp \
../src/EvolutionaryModaber.cpp \
../src/LiftedModaber.cpp \
../src/LiftedTranslator.cpp \
../src/Modaber.cpp \
../src/MyAction.cpp \
../src/MyAssignment.cpp \
../src/MyAtom.cpp \
../src/MyComparison.cpp \
../src/MyEnvironment.cpp \
../src/MyLiftedProposition.cpp \
../src/MyObject.cpp \
../src/MyOperator.cpp \
../src/MyPartialAction.cpp \
../src/MyProblem.cpp \
../src/MyStateVariable.cpp \
../src/MyTimer.cpp \
../src/NumericalPlanningGraph.cpp \
../src/PreconditionFinder.cpp \
../src/ProblemPrinter.cpp \
../src/SimpleModaber.cpp \
../src/SketchyPlan.cpp \
../src/SolutionSimulator.cpp \
../src/Solver.cpp \
../src/Translator.cpp \
../src/Utilities.cpp \
../src/modaberMain.cpp 

OBJS += \
./src/Agent.o \
./src/CVC4Problem.o \
./src/EvolutionaryModaber.o \
./src/LiftedModaber.o \
./src/LiftedTranslator.o \
./src/Modaber.o \
./src/MyAction.o \
./src/MyAssignment.o \
./src/MyAtom.o \
./src/MyComparison.o \
./src/MyEnvironment.o \
./src/MyLiftedProposition.o \
./src/MyObject.o \
./src/MyOperator.o \
./src/MyPartialAction.o \
./src/MyProblem.o \
./src/MyStateVariable.o \
./src/MyTimer.o \
./src/NumericalPlanningGraph.o \
./src/PreconditionFinder.o \
./src/ProblemPrinter.o \
./src/SimpleModaber.o \
./src/SketchyPlan.o \
./src/SolutionSimulator.o \
./src/Solver.o \
./src/Translator.o \
./src/Utilities.o \
./src/modaberMain.o 

CPP_DEPS += \
./src/Agent.d \
./src/CVC4Problem.d \
./src/EvolutionaryModaber.d \
./src/LiftedModaber.d \
./src/LiftedTranslator.d \
./src/Modaber.d \
./src/MyAction.d \
./src/MyAssignment.d \
./src/MyAtom.d \
./src/MyComparison.d \
./src/MyEnvironment.d \
./src/MyLiftedProposition.d \
./src/MyObject.d \
./src/MyOperator.d \
./src/MyPartialAction.d \
./src/MyProblem.d \
./src/MyStateVariable.d \
./src/MyTimer.d \
./src/NumericalPlanningGraph.d \
./src/PreconditionFinder.d \
./src/ProblemPrinter.d \
./src/SimpleModaber.d \
./src/SketchyPlan.d \
./src/SolutionSimulator.d \
./src/Solver.d \
./src/Translator.d \
./src/Utilities.d \
./src/modaberMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I../VALfiles/parsing -I.././ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


