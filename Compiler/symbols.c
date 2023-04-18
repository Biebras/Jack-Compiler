
/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Symbol Tables Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Saulius Vincevičius
Student ID: 
Email: sc21sv@leeds.ac.uk
Date Work Commenced: 2022-04-05
*************************************************************************/

#include "symbols.h"

Scope* CreateScope(Symbol* scopeSymbol);
void AddSymbol(Scope* scope, Symbol* symbol);
Symbol* CreateSymbol(char* name, char* type, char* kind, int createSubScope);
void PrintSymbol(Symbol* symbol);
void PrintScope(Scope* scope);
void FreeScope(Scope* scope);

Scope* programScope = NULL;
Scope* currentScope = NULL;

void InitSymbolTable()
{
    Symbol* programSymbol = CreateSymbol("Program", "NULL", "NULL", 1);
    programScope = CreateScope(programSymbol);
    currentScope = programScope;
}

Scope* CreateScope(Symbol* scopeSymbol)
{
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->scopeSymbol = scopeSymbol;
    scope->length = 0;
    scope->parentScope = currentScope;
    return scope;
}

void AddSymbol(Scope* scope, Symbol* symbol)
{
    scope->symbols[scope->length] = symbol;
    scope->length++;
}

Symbol* CreateSymbol(char* name, char* type, char* kind, int createSubScope)
{
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    strcpy(symbol->name, name);
    strcpy(symbol->type, type);
    strcpy(symbol->kind, kind);
    symbol->subScope = NULL;

    if (createSubScope == 1)
    {
        symbol->subScope = CreateScope(symbol);
    }

    return symbol;
}

Scope* CreateClass(char* className, char* type, char* kind)
{
    Symbol* classSymbol = CreateSymbol(className, type, kind, 1);
    Scope* classScope = CreateScope(classSymbol);
    AddSymbol(programScope, classSymbol);
    return classScope;
}

Symbol* CreateSymbolAtScope(Scope* scope, char* name, char* type, char* kind, int createSubScope)
{
    Symbol* symbol = CreateSymbol(name, type, kind, createSubScope);
    AddSymbol(scope, symbol);
    return symbol;
}

Symbol* CreateSymbolAtCurrentScope(char* name, char* type, char* kind, int createSubScope)
{
    return CreateSymbolAtScope(currentScope, name, type, kind, createSubScope);
}

Scope* FindClass(char* className)
{
    Scope* classScope = NULL;

    for (int i = 0; i < programScope->length; i++)
    {
        Symbol* programSymbol = programScope->symbols[i];

        if (strcmp(programSymbol->name, className) == 0)
        {
            return programSymbol->subScope;
        } 
    }

    return NULL;
}

Scope* FindParentClass()
{
    Scope* scope = currentScope;

    while (scope != NULL)
    {
        if (scope->scopeSymbol != NULL && strcmp(scope->scopeSymbol->kind, "class") == 0)
        {
            return scope;
        }

        scope = scope->parentScope;
    }

    return NULL;
}

/// @brief Finds a symbol in the scope and all parent scopes.(Moves UP the tree) (BFS search)
Symbol* FindSymbolUp(Scope* startScope, char* name)
{
    if (startScope == NULL)
        return NULL;

    for (int i = 0; i < startScope->length; i++)
    {
        Symbol* symbol = startScope->symbols[i];

        if (strcmp(symbol->name, name) == 0)
        {
            return symbol;
        }
    }

    Symbol* symbol = FindSymbolUp(startScope->parentScope, name);

    return symbol;
}

/// @brief Finds a symbol in the scope and all child scopes.(Moves DOWN the tree) (BFS search)
Symbol* FindSymbolDown(Scope* startScope, char* name)
{
    if (startScope == NULL)
        return NULL;

    Scope* scopes[200];
    int head = 0;
    int tail = 0;

    scopes[tail++] = startScope;

    while (head < tail) 
    {
        Scope* scope = scopes[head++];

        for (int i = 0; i < scope->length; i++) 
        {
            Symbol* symbol = scope->symbols[i];

            if (strcmp(symbol->name, name) == 0)
                return symbol;

            if (symbol->subScope != NULL) 
            {
                if (tail >= 200)
                    return NULL;

                scopes[tail++] = symbol->subScope;
            }
        }
    }

    return NULL;
}

Symbol* FindSymbolAtCurrentScope(char* name)
{
    return FindSymbolUp(currentScope, name);
}

Symbol* FindGlobalSymbol(char* className, char* name)
{
    Scope* classScope = FindClass(className);

    if (classScope == NULL)
        return NULL;

    Symbol* symbol = FindSymbolUp(classScope, name);
    
    return symbol;
}

int IsUndeclearedSymbol(Symbol* symbol)
{
    if (symbol == NULL)
        return 1;

    if (strcmp(symbol->type, "NULL") == 0 && strcmp(symbol->kind, "NULL") == 0)
        return 1;

    return 0;
}

void ResetCurrentScope()
{
    currentScope = programScope;
}

void EnterScope(Symbol* symbol)
{
    currentScope = symbol->subScope;
}

void ExitScope()
{
    currentScope = currentScope->parentScope;
}

void PrintSymbol(Symbol* symbol)
{
    if (symbol == NULL)
    {
        printf("Symbol is NULL\n");
        return;
    }

    Scope* subScope = symbol->subScope;
    char* subScopeName = subScope == NULL ? "NULL" : subScope->scopeSymbol->name;
    printf("(N: %s, T: %s, K: %s, S: %s)\n", symbol->name, symbol->type, symbol->kind, subScopeName);
}

void PrintScope(Scope* scope)
{
    if (scope == NULL) 
        return;
    
    char* scopeName = scope->scopeSymbol == NULL ? "NULL" : scope->scopeSymbol->name;

    printf("==%s SCOPE START==\n", scopeName);

    if (scope->scopeSymbol != NULL)
    {
        printf("Scope symbol: ");
        PrintSymbol(scope->scopeSymbol);
    }

    printf("Scope length: %d\n\n", scope->length);

    for (int i = 0; i < scope->length; i++)
    {
        Symbol* symbol = scope->symbols[i];
        
        PrintSymbol(symbol);

        if (symbol->subScope != NULL)
        {
            PrintScope(symbol->subScope);
        }
    }

    printf("==%s SCOPE END==\n", scopeName);
}

void PrintSymbolTable()
{
    PrintScope(programScope);
}

void FreeScope(Scope* scope)
{
    if (scope == NULL) 
        return;

    for (int i = 0; i < scope->length; i++) 
    {
        Symbol* symbol = scope->symbols[i];
        FreeScope(symbol->subScope);
        free(symbol);
    }
    
    free(scope);
}

void FreeSymbolTable()
{
    FreeScope(programScope);
}