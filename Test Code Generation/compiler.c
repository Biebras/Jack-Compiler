/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdarg.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "compiler.h"

bool secondPass = false;

char** buildInFiles = NULL;
int buildInFileCount = 0;
char** directoryFiles = NULL;
int directoryFileCount = 0;

FILE* codeFile = NULL;
int whileCount = 0;
int ifCount = 0;

/// @brief Returns all files with .jack extension in the given directory. (From bing chat)
/// @param dirPath Path to the directory.
/// @param numFiles Number of files found.
/// @return Array of file names.
char** getJackFiles(char* dirPath, int* numFiles) 
{
    DIR* dir = opendir(dirPath);

    if (dir == NULL) 
    {
        printf("Could not open directory %s\n", dirPath);
        return NULL;
    }

    struct dirent* entry;
    int count = 0;
    char** files = NULL;

    while ((entry = readdir(dir)) != NULL) 
    {
        char path[1024];
        struct stat info;
        snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);
        if (!stat(path, &info))
        {
            if (S_ISREG(info.st_mode))
            {
                char* ext = strrchr(entry->d_name, '.');

                if (ext && strcmp(ext, ".jack") == 0) 
                {
                    files = realloc(files, sizeof(char*) * (count + 1));
                    files[count] = malloc(strlen(entry->d_name) + 1);
                    strcpy(files[count], entry->d_name);
                    count++;
                }
            }
        }
    }

    closedir(dir);
    *numFiles = count;
    return files;
}

int InitCompiler ()
{
	InitSymbolTable();

	return 1;
}

ParserInfo ParseBuildInFiles()
{
	ParserInfo p;
	p.er = none;

	if(secondPass == true)
		return p;

	for (int i = 0; i < buildInFileCount; i++)
	{
		InitParser(buildInFiles[i]);
		p = Parse();
		StopParser();

		if (p.er != none)
		{
			return p;
		}
	}

	return p;
}

ParserInfo ParseDirectoryFiles(char* dir_name)
{
	ParserInfo p;
	p.er = none;

	for (int i = 0; i < directoryFileCount; i++)
	{
		char filePath[1024];
		snprintf(filePath, sizeof(filePath), "%s/%s", dir_name, directoryFiles[i]);

		if (secondPass == true)
			InitCodeGeneration(filePath);

		InitParser(filePath);
		p = Parse();
		StopParser();

		if (secondPass == true)
			StopCodeGeneration();

		if (p.er != none)
		{
			return p;
		}
	}

	return p;
}

ParserInfo ParseFiles(char* dir_name)
{
	ParserInfo p;
	p.er = none;

	p = ParseBuildInFiles();

	if (p.er != none)
		return p;

	p = ParseDirectoryFiles(dir_name);

	if (p.er != none)
		return p;

	// If second pass is true, then we don't need to check for undeclared symbols.
	if (secondPass == true)
		return p;

	Symbol* undeclared = SearchForUndeclaredSymbol();

	if (undeclared != NULL)
	{
		Token token = undeclared->pi.tk;
		char* errorMessage = "Undeclared symbol";
		printf("Error: %s. Accured at line %d near %s token in file %s.\n", errorMessage, token.ln, token.lx, token.fl);
		return undeclared->pi;
	}

	return p;
}

ParserInfo compile (char* dir_name)
{
	printf("Starting compiler...\n");

	ParserInfo p;
	p.er = none;

	buildInFiles = getJackFiles(".", &buildInFileCount);
	directoryFiles = getJackFiles(dir_name, &directoryFileCount);

	secondPass = false;

	p = ParseFiles(dir_name);

	if (p.er != none)
		return p;

	secondPass = true;

	p = ParseFiles(dir_name);

	if (p.er != none)
		return p;

	return p;
}

int StopCompiler ()
{
	//PrintSymbolTable();
	printf("Compiler stopped.\n");
	FreeSymbolTable();
	return 1;
}

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

    if(strcmp(symbol->kind, "static") == 0)
    {
        EmitCode("pop static %d\n", address);
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

    if(strcmp(symbol->kind, "static") == 0)
    {
        EmitCode("push static %d\n", address);
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

int EmitStartWhile1()
{
    int whileIndex = whileCount;
    EmitCode("label WHILE_EXP%d\n", whileIndex);
    whileCount++;
    return whileIndex;
}

void EmitStartWhile2(int whileIndex)
{
    EmitCode("not\n");
    EmitCode("if-goto WHILE_END%d\n", whileIndex);
}

void EmitEndWhile(int whileIndex)
{
    EmitCode("goto WHILE_EXP%d\n", whileIndex);
    EmitCode("label WHILE_END%d\n", whileIndex);
}

int EmitIfStart()
{
    int ifIndex = ifCount;

    EmitCode("if-goto IF_TRUE%d\n", ifIndex);
    EmitCode("goto IF_FALSE%d\n", ifIndex);
    EmitCode("label IF_TRUE%d\n", ifIndex);
    ifCount++;

    return ifIndex;
}

void EmitIfEnd(int ifIndex)
{
    EmitCode("label IF_FALSE%d\n", ifIndex);
}

void EmitElseStart(int ifIndex)
{
    EmitCode("goto IF_END%d\n", ifIndex);
    EmitCode("label IF_FALSE%d\n", ifIndex);
}

void EmitElseEnd(int ifIndex)
{
    EmitCode("label IF_END%d\n", ifIndex);
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
