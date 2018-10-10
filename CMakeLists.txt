cmake_minimum_required(VERSION 3.10)
project(Modaber)

# set(CMAKE_CXX_STANDARD 14)
include(VALfiles/parsing/CMakeLists.txt)

include_directories(Lingeling)
include_directories(src)
include_directories(src/Deprecated)
include_directories(VALfiles)

add_executable(Modaber
        Lingeling/lglib.c
        Lingeling/lglib.h
#         src/Deprecated/NumericRPG.cpp
#         src/Deprecated/NumericRPG.h
#         src/Deprecated/OldMyAnalyzer.h
        src/Translation/Translator.cpp
        src/Agent.cpp
        src/Agent.h
        src/CVC4Problem.cpp
        src/CVC4Problem.h
        src/EvolutionaryModaber.cpp
        src/EvolutionaryModaber.h
        src/LiftedModaber.cpp
        src/LiftedModaber.h
        src/LiftedTranslator.cpp
        src/LiftedTranslator.h
        src/Modaber.cpp
        src/Modaber.h
        src/modaberMain.cpp
        src/MyAction.cpp
        src/MyAction.h
        src/MyAssignment.cpp
        src/MyAssignment.h
        src/MyAtom.cpp
        src/MyAtom.h
        src/MyComparison.cpp
        src/MyComparison.h
        src/MyEnvironment.cpp
        src/MyEnvironment.h
        src/MyLiftedProposition.cpp
        src/MyLiftedProposition.h
        src/MyObject.cpp
        src/MyObject.h
        src/MyOperator.cpp
        src/MyOperator.h
        src/MyPartialAction.cpp
        src/MyPartialAction.h
        src/MyProblem.cpp
        src/MyProblem.h
        src/MyStateVariable.cpp
        src/MyStateVariable.h
        src/MyTimer.cpp
        src/MyTimer.h
        src/NumericalPlanningGraph.cpp
        src/NumericalPlanningGraph.h
        src/PreconditionFinder.cpp
        src/PreconditionFinder.h
        src/ProblemPrinter.cpp
        src/ProblemPrinter.h
        src/SimpleModaber.cpp
        src/SimpleModaber.h
        src/SketchyPlan.cpp
        src/SketchyPlan.h
        src/SolutionSimulator.cpp
        src/SolutionSimulator.h
        src/Solver.cpp
        src/Solver.h
        src/Translator.cpp
        src/Translator.h
        src/Utilities.cpp
        src/Utilities.h
#        VALfiles/parsing/DebugWriteController.cpp
#        VALfiles/parsing/DebugWriteController.h
#        VALfiles/parsing/fixyywrap.cpp
#        VALfiles/parsing/fixyywrap.h
#        VALfiles/parsing/lex.yy.cc
#        VALfiles/parsing/macros.h
#        VALfiles/parsing/parse_error.h
#        VALfiles/parsing/pddl+.cpp
#        VALfiles/parsing/pddl+.lex
#        VALfiles/parsing/pddl+.yacc
#        VALfiles/parsing/ptree.cpp
#        VALfiles/parsing/ptree.h
#        VALfiles/parsing/sStack.h
#        VALfiles/parsing/VisitController.h
#        VALfiles/parsing/WriteController.h
        VALfiles/Cascader.h
        VALfiles/Environment.cpp
        VALfiles/Environment.h
        VALfiles/Exceptions.h
        VALfiles/FastEnvironment.cpp
        VALfiles/FastEnvironment.h
        VALfiles/FuncAnalysis.cpp
        VALfiles/FuncAnalysis.h
        VALfiles/instantiation.cpp
        VALfiles/instantiation.h
        VALfiles/main.h
        VALfiles/Partitions.h
        VALfiles/SimpleEval.cpp
        VALfiles/SimpleEval.h
        VALfiles/TIM.cpp
        VALfiles/TIM.h
        VALfiles/TimSupport.cpp
        VALfiles/TimSupport.h
        VALfiles/TIMUtilities.h
        VALfiles/typecheck.cpp
        VALfiles/typecheck.h
        VALfiles/TypedAnalyser.cpp
        VALfiles/TypedAnalyser.h
        VALfiles/TypeStripWC.cpp
        VALfiles/TypeStripWC.h)

target_include_directories(Modaber PUBLIC ./ VALfiles/parsing)
target_link_libraries(Modaber gmp cvc4 ${CMAKE_CURRENT_BINARY_DIR}/libParsePDDL.a)
