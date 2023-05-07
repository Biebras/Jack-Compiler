#ifndef COMPILER_H
#define COMPILER_H

#define TEST_COMPILER    // uncomment to run the compiler autograder

#include "parser.h"
#include "symbols.h"

int InitCompiler ();
ParserInfo compile (char* dir_name);
int StopCompiler();
int InitCodeGeneration(char* filename);
void EmitCode(const char *format, ...);
void EmitPushConstant(int value);
void EmitPushLocal(int index);
void EmitPopPointer(int index);
void EmitPopTemp(int index);
void EmitPopLocal(int index);
void EmitPop(Symbol* symbol);
void EmitPush(Symbol* symbol);
void EmitConstructor(Symbol* symbol);
void EmitMethod(Symbol* symbol);
void EmitFunction1(Symbol* symbol);
void EmitFunction2(char* className, char* functionName, int argumentCount);
void EmitCaller(Symbol* caller);
void EmitCall1(Symbol* symbol);
void EmitCall2(char* className, char* functionName, int argumentCount);
int EmitStartWhile1();
void EmitStartWhile2(int whileIndex);
void EmitEndWhile(int whileIndex);
int EmitIfStart();
void EmitElseStart(int ifIndex);
void EmitElseEnd(int ifIndex);
void EmitIfEnd(int idIndex);
void EmitString(char* string);
void EmitLetArray(int arrayAdress);
void EmitAccessArray(int arrayAdress);
void EmitDivide();
void EmitMultiply();
void EmitReturn();
void StopCodeGeneration();

#endif
