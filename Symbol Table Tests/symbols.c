
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
#include "parser.h"

Scope* CreateScope(Symbol* scopeSymbol, Scope* parentScope);
void AddSymbol(Scope* scope, Symbol* symbol);
Symbol* CreateSymbol(char* name, char* type, char* kind, Scope* parentScope, ParserInfo pi, int createSubScope);
Symbol* FindSymbolAtScope(Scope* scope, char* name);
void PrintScopeTabs(int scopeLevel);
void PrintSymbol(Symbol* symbol);
void PrintScope(Scope* scope);
void FreeScope(Scope* scope);

Scope* programScope = NULL;
Scope* currentScope = NULL;

void InitSymbolTable()
{
    ParserInfo pi;
    Token token;
    pi.tk = token;
    Symbol* programSymbol = CreateSymbol("Program", "Program", "Program", NULL, pi, 1);
    programScope = CreateScope(programSymbol, NULL);
    currentScope = programScope;
}

Scope* GetCurrentScope()
{
    return currentScope;
}

Scope* CreateScope(Symbol* scopeSymbol, Scope* parentScope)
{
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->scopeSymbol = scopeSymbol;
    scope->length = 0;
    scope->parentScope = parentScope;

    if(parentScope == NULL)
        scope->scopeLevel = 0;
    else
        scope->scopeLevel = parentScope->scopeLevel + 1;

    return scope;
}

int GetSymbolAddress(Symbol* symbol)
{
    int count = 0;
    Scope* scope = symbol->parentScope;

    for (int i = 0; i < scope->length; i++)
    {
        Symbol* s = scope->symbols[i];

        if (s == symbol)
            continue;

        if (strcmp(scope->symbols[i]->kind, symbol->kind) == 0)
            count++;
    }

    return count;
}

void AddSymbol(Scope* scope, Symbol* symbol)
{
    // Assign address
    symbol->parentScope = scope;
    int address = GetSymbolAddress(symbol);
    symbol->address = address;

    // Add symbol to scope
    scope->symbols[scope->length] = symbol;
    scope->length++;
}

Symbol* CreateSymbol(char* name, char* type, char* kind, Scope* parentScope, ParserInfo pi, int createSubScope)
{
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    strcpy(symbol->name, name);
    strcpy(symbol->type, type);
    strcpy(symbol->kind, kind);
    symbol->subScope = NULL;
    symbol->pi = pi;

    if (createSubScope == 1)
    {
        symbol->subScope = CreateScope(symbol, parentScope);
    }

    return symbol;
}

Scope* CreateClass(char* className, char* type, char* kind, ParserInfo pi)
{
    Symbol* classSymbol = CreateSymbol(className, type, kind, programScope, pi, 1);
    AddSymbol(programScope, classSymbol);
    return classSymbol->subScope;
}

Symbol* CreateSymbolAtScope(Scope* scope, char* name, char* type, char* kind, ParserInfo pi, int createSubScope)
{
    Symbol* symbol = CreateSymbol(name, type, kind, scope, pi, createSubScope);
    AddSymbol(scope, symbol);
    return symbol;
}

Symbol* CreateSymbolAtCurrentScope(char* name, char* type, char* kind, ParserInfo pi, int createSubScope)
{
    return CreateSymbolAtScope(currentScope, name, type, kind, pi, createSubScope);
}

Symbol* FindSymbolAtScope(Scope* scope, char* name)
{
    for (int i = 0; i < scope->length; i++)
    {
        Symbol* symbol = scope->symbols[i];

        if (strcmp(symbol->name, name) == 0)
        {
            return symbol;
        }
    }

    return NULL;
}

Symbol* FindSymbolAtCurrentScope(char* name)
{
    return FindSymbolAtScope(currentScope, name);
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
        if(scope->parentScope == programScope)
            return scope;

        scope = scope->parentScope;
    }

    return NULL;
}

/// @brief Finds a symbol in the scope and all parent scopes.(Moves UP the tree) (BFS search)
Symbol* SearchSymbolUp(Scope* startScope, char* name)
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

    Symbol* symbol = SearchSymbolUp(startScope->parentScope, name);

    return symbol;
}

/// @brief Finds a symbol in the scope and all child scopes.(Moves DOWN the tree) (BFS search)
Symbol* SearchForUndeclaredSymbol()
{
    Scope* scopes[1000];
    int head = 0;
    int tail = 0;

    scopes[tail++] = programScope;

    while (head < tail) 
    {
        Scope* scope = scopes[head++];

        for (int i = 0; i < scope->length; i++) 
        {
            Symbol* symbol = scope->symbols[i];

            if (strcmp(symbol->type, "NULL") == 0 || strcmp(symbol->kind, "NULL") == 0)
            {
                symbol->pi.er = undecIdentifier;
                return symbol;
            }

            if (symbol->subScope != NULL) 
            {
                if (tail >= 1000)
                    return NULL;

                scopes[tail++] = symbol->subScope;
            }
        }
    }

    return NULL;
}

Symbol* SearchSymbolFromCurrentScope(char* name)
{
    return SearchSymbolUp(currentScope, name);
}

Symbol* SearchGlobalSymbol(char* className, char* name)
{
    Scope* classScope = FindClass(className);

    if (classScope == NULL)
        return NULL;

    Symbol* symbol = SearchSymbolUp(classScope, name);
    
    return symbol;
}

int IsUndeclearedSymbol(Symbol* symbol)
{
    if (symbol == NULL)
        return 1;

    if (strcmp(symbol->type, "NULL") == 0 || strcmp(symbol->kind, "NULL") == 0)
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

void PrintScopeTabs(int scopeLevel)
{
    for (int i = 0; i < scopeLevel; i++)
    {
        printf("\t");
    }
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

    if (strcmp(symbol->type, "NULL") == 0 || strcmp(symbol->kind, "NULL") == 0)
    {
        printf("(N: %s, T: %s, K: %s, A: %d, S: %s) <--- UNDECLARED\n", symbol->name, symbol->type, symbol->kind, symbol->address, subScopeName);
        return;
    }
    
    printf("(N: %s, T: %s, K: %s, A: %d, S: %s)\n", symbol->name, symbol->type, symbol->kind, symbol->address, subScopeName);
}

void PrintScope(Scope* scope)
{
    if (scope == NULL)
        return;

    PrintScopeTabs(scope->scopeLevel);
    printf("{\n");

    for (int i = 0; i < scope->length; i++)
    {
        Symbol* symbol = scope->symbols[i];
        
        PrintScopeTabs(scope->scopeLevel + 1);
        PrintSymbol(symbol);

        PrintScope(symbol->subScope);
    }

    PrintScopeTabs(scope->scopeLevel);
    printf("}\n");
}

void PrintSymbolTable()
{
    PrintSymbol(programScope->scopeSymbol);
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