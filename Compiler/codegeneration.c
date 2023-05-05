#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "codegeneration.h"

FILE* file;

int InitCodeGeneration(char* filename)
{
    file = fopen(filename, "w");

    if (file == 0)
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
    vfprintf(file, format, args);
    va_end(args);
}

void EmitPushConstant(int value)
{
    EmitCode("push constant %d\n", value);
}

void EmitPopTemp(int index)
{
    EmitCode("pop temp %d\n", index);
}

void EmitFunction(char* name, int localVariablesCount)
{
    EmitCode("function %s %d\n", name, localVariablesCount);
}

void EmitCall(char* name, int localVariablesCount)
{
    EmitCode("call %s %d\n", name, localVariablesCount);
}

void EmitString(char* string)
{
    int length = strlen(string);

    EmitPushConstant(length);
    EmitCall("String.new", 1);

    for (int i = 0; i < length; i++)
    {
        EmitPushConstant(string[i]);
        EmitCall("String.appendChar", 2);
    }   
}

void StopCodeGeneration()
{
    fclose(file);
}
