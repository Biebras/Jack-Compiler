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
    Scope* parentScope;
    Scope* subScope;
    ParserInfo pi;
    int adress;
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
int GetSymbolAddress(Symbol* symbol);
int GetArgumentCount(Symbol* symbol);
Scope* GetCurrentScope();
Scope* CreateClass(char* className, char* type, char* kind, ParserInfo pi);
Symbol* CreateSymbolAtScope(Scope* scope, char* name, char* type, char* kind, ParserInfo pi, int createSubScope);
Symbol* CreateSymbolAtCurrentScope(char* name, char* type, char* kind, ParserInfo pi, int createSubScope);
Symbol* FindSymbolAtCurrentScope(char* name);
Scope* FindClass(char* className);
Scope* FindParentClass();
Symbol* SearchSymbolFromCurrentScope(char* name);
Symbol* SearchGlobalSymbol(char* className, char* name);
Symbol* SearchForUndeclaredSymbol();
int IsUndeclearedSymbol(Symbol* symbol);
void ResetCurrentScope();
void EnterScope(Symbol* symbol);
void ExitScope();
void PrintSymbolTable();
void FreeSymbolTable();

#endif