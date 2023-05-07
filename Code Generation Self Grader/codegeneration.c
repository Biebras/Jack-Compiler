#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "codegeneration.h"

FILE* codeFile = NULL;
int whileCount = 0;
int ifCount = 0;

int InitCodeGeneration(char* filename)
{
    // Change .jack extension to .vm
    char newFilename[1024];
    strcpy(newFilename, filename);
    char *dot = strrchr(newFilename, '.');
    strcpy(dot, ".vm");

    codeFile = fopen(newFilename, "w");

    if (codeFile == 0)
    {
        printf("Error: can't open file\n");
        return 1;
    }

    return 0;
}

void EmitCode(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vfprintf(codeFile, format, args);
    va_end(args);

    if (result < 0) {
        printf("Error writing to file: ");
        if (ferror(codeFile)) {
            perror("");
        } else {
            printf("Unknown error\n");
        }
    }
}

void EmitPushStatic(int index)
{
    EmitCode("push static %d\n", index);
}

void EmitPushConstant(int value)
{
    EmitCode("push constant %d\n", value);
}

void EmitPushLocal(int index)
{
    EmitCode("push local %d\n", index);
}

void EmitPopPointer(int index)
{
    EmitCode("pop pointer %d\n", index);
}

void EmitPopTemp(int index)
{
    EmitCode("pop temp %d\n", index);
}

void EmitPopLocal(int index)
{
    EmitCode("pop local %d\n", index);
}

void EmitPop(Symbol* symbol)
{
    Symbol* parentSymbol = symbol->parentScope->scopeSymbol;
    Symbol* scopeSymbol = GetCurrentScope()->scopeSymbol;
    int address = symbol->address;

    if(strcmp(symbol->name, "this") == 0)
    {
        EmitCode("pop pointer 0\n");
        return;
    }

    if(strcmp(symbol->kind, "argument") == 0)
    {
        EmitCode("pop argument %d\n", address);
        return;
    }

    if(strcmp(parentSymbol->kind, "class") == 0)
    {
        EmitCode("pop this %d\n", address);
        return;
    }

    EmitPopLocal(address);
}

void EmitPush(Symbol* symbol)
{
    Symbol* parentSymbol = symbol->parentScope->scopeSymbol;
    Symbol* scopeSymbol = GetCurrentScope()->scopeSymbol;
    int address = symbol->address;

    if(strcmp(symbol->name, "this") == 0)
    {
        EmitCode("push pointer 0\n");
        return;
    }

    if(strcmp(symbol->kind, "argument") == 0)
    {
        EmitCode("push argument %d\n", address);
        return;
    }

    if(strcmp(parentSymbol->kind, "class") == 0)
    {
        EmitCode("push this %d\n", address);
        return;
    }

    EmitPushLocal(address);
}

void EmitConstructor(Symbol* symbol)
{
    int globalVarCount = GetGlobalVarCount(symbol->parentScope);

    EmitFunction1(symbol);
    EmitPushConstant(globalVarCount);
    EmitCall2("Memory", "alloc", 1);
    EmitPopPointer(0);
}

void EmitMethod(Symbol* symbol)
{
    EmitFunction1(symbol);
    EmitCode("push argument 0\n");
    EmitPopPointer(0);
}

void EmitFunction1(Symbol* symbol)
{
    char* className = symbol->parentScope->scopeSymbol->name;
    char* functionName = symbol->name;
    int argumentCount = GetLocalVarCount(symbol);

    EmitFunction2(className, functionName, argumentCount);
}

void EmitFunction2(char* className, char* functionName, int argumentCount)
{
    whileCount = 0;
    ifCount = 0;

    EmitCode("function %s.%s %d\n", className, functionName, argumentCount);
}

void EmitCaller(Symbol* caller)
{
    if(caller == NULL)
    {
        Symbol* scopeSymbol = GetCurrentScope()->scopeSymbol;

        if(strcmp(scopeSymbol->kind, "method") == 0)
            EmitCode("push pointer 0\n");
        else if (strcmp(scopeSymbol->kind, "constructor") == 0)
            EmitCode("push pointer 0\n");
    }

    if(caller != NULL)
    {
        if(strcmp(caller->kind, "class") != 0)
            EmitPush(caller);
    }
}

void EmitCall1(Symbol* symbol)
{
    char* className = symbol->parentScope->scopeSymbol->name;
    char* functionName = symbol->name;
    int argumentCount = GetArgumentCount(symbol);
    
    EmitCall2(className, functionName, argumentCount);
}

void EmitCall2(char* className, char* functionName, int argumentCount)
{
    EmitCode("call %s.%s %d\n", className, functionName, argumentCount);
}

void EmitStartWhile1()
{
    EmitCode("label WHILE_EXP%d\n", whileCount);
    whileCount++;
}

void EmitStartWhile2()
{
    EmitCode("not\n");
    EmitCode("if-goto WHILE_END%d\n", whileCount - 1);
}

void EmitEndWhile()
{
    EmitCode("goto WHILE_EXP%d\n", whileCount - 1);
    EmitCode("label WHILE_END%d\n", whileCount - 1);
}

void EmitStartIf()
{
    EmitCode("if-goto IF_TRUE%d\n", ifCount);
    EmitCode("goto IF_FALSE%d\n", ifCount);
    EmitCode("label IF_TRUE%d\n", ifCount);
    ifCount++;
}

void EmitEndIf()
{
    EmitCode("label IF_FALSE%d\n", ifCount - 1);
}

void EmitString(char* string)
{
    int length = strlen(string);

    EmitPushConstant(length);
    EmitCall2("String", "new", 1);

    for (int i = 0; i < length; i++)
    {
        EmitPushConstant(string[i]);
        EmitCall2("String", "appendChar", 2);
    }   
}

void EmitLetArray(int arrayAdress)
{
    EmitCode("pop temp 0\n");
    EmitCode("pop pointer 1\n");
    EmitCode("push temp 0\n");
    EmitCode("pop that %d\n", arrayAdress);
}

void EmitAccessArray(int arrayAdress)
{
    EmitPushLocal(arrayAdress);
    EmitCode("add\n");
    EmitCode("pop pointer 1\n");
    EmitCode("push that %d\n", arrayAdress);
}

void EmitDivide()
{
    EmitCall2("Math", "divide", 2);
}

void EmitMultiply()
{
    EmitCall2("Math", "multiply", 2);
}

void EmitReturn()
{
    EmitCode("return\n");
}

void StopCodeGeneration()
{
    fclose(codeFile);
}
