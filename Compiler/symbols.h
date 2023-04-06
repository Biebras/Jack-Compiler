#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

// define your own types and function prototypes for the symbol table(s) module below

typedef struct Scope Scope;

typedef struct
{
    char name[128];
    char type[32];
    char kind[32];
    Scope* subScope;
} Symbol;

struct Scope
{
    Symbol* scopeSymbol;
    Symbol* symbols[200];
    int length;
    Scope* parentScope;
};

void InitSymbolTable();
Symbol* CreateSymbolAtCurrentScope(char* name, char* type, char* kind);
Symbol* CreateSymbolWithSubScopeAtCurrentScope(char* name, char* type, char* kind);
Symbol* FindSymbolFromCurrentScope(char* name, char* type, char* kind);
Symbol* FindSymbolInClass(char* className, char* name, char* type, char* kind);
Symbol* FindParentClass();
void ResetCurrentScope();
void EnterScope(Symbol* symbol);
void ExitScope();
void PrintSymbolTable();
void FreeSymbolTable();

#endif