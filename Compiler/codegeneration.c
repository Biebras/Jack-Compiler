#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "symbols.h"
#include "codegeneration.h"

FILE* code_file = NULL;

int InitCodeGeneration(char* filename)
{
    // Change .jack extension to .vm
    char newFilename[1024];
    strcpy(newFilename, filename);
    char *dot = strrchr(newFilename, '.');
    strcpy(dot, ".vm");

    code_file = fopen(newFilename, "w");

    if (code_file == 0)
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
    int result = vfprintf(code_file, format, args);
    va_end(args);

    if (result < 0) {
        printf("Error writing to file: ");
        if (ferror(code_file)) {
            perror("");
        } else {
            printf("Unknown error\n");
        }
    }
}

void EmitPushConstant(int value)
{
    EmitCode("push constant %d\n", value);
}

void EmitPopPointer(int index)
{
    EmitCode("pop pointer %d\n", index);
}

void EmitPopTemp(int index)
{
    EmitCode("pop temp %d\n", index);
}

void EmitConstructor(char* name, int localFieldsCount)
{
    EmitPushConstant(localFieldsCount);
    //EmitCall("Memory.alloc", 1);
    EmitPopPointer(0);
}

void EmitFunction(Symbol* symbol)
{
    char* className = symbol->parentScope->scopeSymbol->name;
    char* functionName = symbol->name;
    int argumentCount = GetArgumentCount(symbol);

    EmitCode("function %s.%s %d\n", className, functionName, argumentCount);
}

void EmitCall1(Symbol* symbol)
{
    char* className = symbol->parentScope->scopeSymbol->name;
    char* functionName = symbol->name;
    int argumentCount = GetArgumentCount(symbol);

    EmitCode("call %s.%s %d\n", className, functionName, argumentCount);
}

void EmitCall2(char* className, char* functionName, int argumentCount)
{
    EmitCode("call %s.%s %d\n", className, functionName, argumentCount);
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

void EmitReturn(Symbol* symbol)
{
    if (symbol == NULL)
    {
        EmitCode("push constant 0\n");
        EmitCode("return\n");
        return;
    }

    EmitCode("return\n");
}

void StopCodeGeneration()
{
    fclose(code_file);
}
