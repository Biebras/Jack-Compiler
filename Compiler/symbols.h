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
    int scopeLevel;
    Scope* parentScope;
};

void InitSymbolTable();
Scope* CreateClass(char* className, char* type, char* kind);
Symbol* CreateSymbolAtScope(Scope* scope, char* name, char* type, char* kind, int createSubScope);
Symbol* CreateSymbolAtCurrentScope(char* name, char* type, char* kind, int createSubScope);
Scope* FindClass(char* className);
Scope* FindParentClass();
Symbol* SearchSymbolFromCurrentScope(char* name);
Symbol* SearchGlobalSymbol(char* className, char* name);
int IsUndeclearedSymbol(Symbol* symbol);
void ResetCurrentScope();
void EnterScope(Symbol* symbol);
void ExitScope();
void PrintSymbolTable();
void FreeSymbolTable();

#endif