#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "compiler.h"
#include "codegeneration.h"

void Error(ParserInfo* parserInfo, Token* token, SyntaxErrors syntaxError, char* errorMessage);
static void PrintErrorCombine(char* errorMessage, ParserInfo* parserInfo);
Token GetNextTokenWithErrorCheck(ParserInfo *pi);
Token PeekNextTokenWithErrorCheck(ParserInfo *pi);
Symbol* DeclareSymbol(Symbol* symbol, char* name, char* type, char* kind, ParserInfo pi, int createSubScope);
ParserInfo ClassDeclar();
ParserInfo MemberDeclar();
ParserInfo ClassVarDeclar();
ParserInfo Type();
ParserInfo SubroutineDeclar();
ParserInfo ParamList();
ParserInfo SubroutineBody();
ParserInfo Statement();
ParserInfo VarDeclarStatement();
ParserInfo LetStatement();
ParserInfo IfStatement();
ParserInfo WhileStatement();
ParserInfo DoStatement();
ParserInfo SubroutineCall();
ParserInfo ExpressionList();
ParserInfo ReturnStatement();
ParserInfo Expression();
ParserInfo RelationalExpression();
ParserInfo ArithmeticExpression();
ParserInfo Term();
ParserInfo Factor();
ParserInfo Operand();
ParserInfo OperantIdentifier();

#define NEXT_TOKEN t = GetNextTokenWithErrorCheck(&pi); if(t.tp == ERR){TokenError(&pi, &t); return pi;}
#define PEEK_TOKEN t = PeekNextTokenWithErrorCheck(&pi); if(t.tp == ERR){TokenError(&pi, &t); return pi;}

extern bool secondPass;

// This code was taken from bing chat
void safe_snprintf(char *buffer, size_t bufferSize, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, bufferSize, format, args);
    va_end(args);

    if (length >= bufferSize) {
        buffer[bufferSize - 1] = '\0';
    }
}

// you can declare prototypes of parser functions below
void Error(ParserInfo* parserInfo, Token* token, SyntaxErrors syntaxError, char* errorMessage)
{
	parserInfo->er = syntaxError;
	parserInfo->tk = *token;
	PrintErrorCombine(errorMessage, parserInfo);
}

void TokenError(ParserInfo* parserInfo, Token* token)
{
	parserInfo->er = lexerErr;
	parserInfo->tk = *token;
	printf("%s at line %d in file %s.", token->lx, token->ln, token->fl);
}

static void PrintErrorCombine(char* errorMessage, ParserInfo* parserInfo)
{
	Token token = parserInfo->tk;
	printf("Error: %s. Accured at line %d near %s token in file %s.\n", errorMessage, token.ln, token.lx, token.fl);
}

Token GetNextTokenWithErrorCheck(ParserInfo *pi)
{
    Token t = GetNextToken();

    if (t.tp == ERR)
    {
        TokenError(pi, &t);
    }

	pi->tk = t;
    return t;
}

Token PeekNextTokenWithErrorCheck(ParserInfo *pi)
{
	Token t = PeekNextToken();

	if (t.tp == ERR)
	{
		TokenError(pi, &t);
	}

	return t;
}

Symbol* DeclareSymbol(Symbol* symbol, char* name, char* type, char* kind, ParserInfo pi, int createSubScope)
{
	if (symbol == NULL)
	{
		symbol = CreateSymbolAtCurrentScope(name, type, kind, pi, createSubScope);

		return symbol;
	}
	
	if (IsUndeclearedSymbol(symbol))
	{
		strcpy(symbol->type, type);
		strcpy(symbol->kind, kind);
		symbol->address = GetSymbolAddress(symbol);
		symbol->pi = pi;
		return symbol;
	}

	// Return symbol on second pass to not confuse with undeclared symbol
	if(secondPass == true)
		return symbol;

	return NULL;
}

int InitParser (char* file_name)
{
	InitLexer(file_name);
	return 1;
}

ParserInfo Parse ()
{
	ParserInfo pi;
	pi.er = none;

	pi = ClassDeclar();

	if (pi.er == none)
		printf("Parsing completed successfully!\n");

	return pi;
}

ParserInfo ClassDeclar()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN
	
	// Check if class keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "class") != 0)
	{
		Error(&pi, &t, classExpected, "class keyword expected");
		return pi;
	}

	NEXT_TOKEN

	// Check if class name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "class name expected");
		return pi;
	}

	// Create a scope for this class
	Symbol* classSymbol = SearchSymbolFromCurrentScope(t.lx);
	classSymbol = DeclareSymbol(classSymbol, t.lx, t.lx, "class", pi, 1);

	if(classSymbol == NULL)
	{
		Error(&pi, &t, redecIdentifier, "class name already exists");
		return pi;
	}

	NEXT_TOKEN

	// Check if open brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

	// Enter class scope
	EnterScope(classSymbol);

	PEEK_TOKEN

	// Can have many member declarations
	// loop while t is not }
	while (t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		pi = MemberDeclar();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
	}
	
	NEXT_TOKEN

	//Cheack if close brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		Error(&pi, &t, closeBraceExpected, "} expected");
		return pi;
	}

	ExitScope();

	return pi;
}

ParserInfo MemberDeclar()
{
	Token t;
	ParserInfo pi;
	pi.er = none;
	
	PEEK_TOKEN

	if(t.tp != RESWORD)
	{
		Error(&pi, &t, memberDeclarErr, "class member declaration must begin with static, field, constructor , function or method keyword");
		return pi;
	}

	if(strcmp(t.lx, "constructor") == 0 || strcmp(t.lx, "function") == 0 || strcmp(t.lx, "method") == 0)
	{
		pi = SubroutineDeclar();

		if (pi.er != none)
			return pi;
	}
	else if(strcmp(t.lx, "static") == 0 || strcmp(t.lx, "field") == 0)
	{
		pi = ClassVarDeclar();

		if (pi.er != none)
			return pi;
	}
	else
	{
		Error(&pi, &t, memberDeclarErr, "class member declaration must begin with static, field, constructor , function or method keyword");
		return pi;
	}

	return pi;
}

ParserInfo ClassVarDeclar()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Store kind of variable
	char kind[128];
	strcpy(kind, t.lx);

	// Check if static or field is found
	if(t.tp != RESWORD || ((strcmp(t.lx, "static") != 0 && strcmp(t.lx, "field") != 0)))
	{
		Error(&pi, &t, classVarErr, "class variable declaration must begin with static or field keyword");
		return pi;
	}

	pi = Type();

	if (pi.er != none)
		return pi;

	// Store type of variable
	char type[128];
	strcpy(type, pi.tk.lx);

	NEXT_TOKEN

	// Check if identifier is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "class variable name expected");
		return pi;
	}

	Symbol* symbol = SearchSymbolFromCurrentScope(t.lx);
	symbol = DeclareSymbol(symbol, t.lx, type, kind, pi, 0);

	if(symbol == NULL)
	{
		Error(&pi, &t, redecIdentifier, "variable name already exists");
		return pi;
	}

	NEXT_TOKEN

	// loop while next token is comma
	while (t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		NEXT_TOKEN

		// Check if identifier is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "class variable name expected");
			return pi;
		}

		// Check if identifier is found
		symbol = SearchSymbolFromCurrentScope(t.lx);
		symbol = DeclareSymbol(symbol, t.lx, type, kind, pi, 0);

		if(symbol == NULL)
		{
			Error(&pi, &t, redecIdentifier, "variable name already exists");
			return pi;
		}

		NEXT_TOKEN
	}

	// Check if semicolon is found
	if(t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	return pi;
}

ParserInfo Type()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if type is found
	if(t.tp != RESWORD && t.tp != ID)
	{
		Error(&pi, &t, illegalType, "type expected");
		return pi;
	}

	// Check if type is int, char or boolean
	if(t.tp == RESWORD)
	{
		if(strcmp(t.lx, "int") != 0 && strcmp(t.lx, "char") != 0 && strcmp(t.lx, "boolean") != 0)
		{
			Error(&pi, &t, illegalType, "type expected");
			return pi;
		}
	}

	// If type is identifier, then it's a class name
	if (t.tp == ID)
	{
		Scope* classScope = FindClass(t.lx);

		if (classScope == NULL)
		{
			//Create undeclared class
			CreateClass(t.lx, t.lx, "NULL", pi);
		}
	}

	return pi;
}

ParserInfo SubroutineDeclar()
{
	Token t;
	ParserInfo pi;
	pi.er = none;
	
	NEXT_TOKEN

	if(t.tp != RESWORD)
	{
		Error(&pi, &t, subroutineDeclarErr, "subroutine declaration must begin with constructor, function or method keyword");
		return pi;
	}

	if(strcmp(t.lx, "constructor") != 0 && strcmp(t.lx, "function") != 0 && strcmp(t.lx, "method") != 0)
	{
		Error(&pi, &t, subroutineDeclarErr, "subroutine declaration must begin with constructor, function or method keyword");
		return pi;
	}

	char kind[128];
	strcpy(kind, t.lx);

	PEEK_TOKEN

	// Check if void or type is found
	if(t.tp != RESWORD || strcmp(t.lx, "void") != 0)
	{
		pi = Type();

		if (pi.er != none)
			return pi;
	}
	else if(t.tp == RESWORD && strcmp(t.lx, "void") == 0)
	{
		NEXT_TOKEN
	}
	else
	{
		Error(&pi, &t, subroutineDeclarErr, "subroutine declaration must begin with constructor, function or method keyword");
		return pi;
	}

	// Store type of subroutine
	char type[128];
	strcpy(type, t.lx);
	
	NEXT_TOKEN

	// Check if subroutine name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "subroutine name expected");
		return pi;
	}
	
	// Find the symbol in the current scope
	Symbol* symbol = SearchSymbolFromCurrentScope(t.lx);
	symbol = DeclareSymbol(symbol, t.lx, type, kind, pi, 1);

	if(symbol == NULL)
	{
		char errorMsg[128];
		safe_snprintf(errorMsg, sizeof(errorMsg), "subroutine (%s) already exists", t.lx);

		Error(&pi, &t, redecIdentifier, errorMsg);
		return pi;
	}

	if(secondPass == true)
	{
		if(strcmp(kind, "method") == 0)
			EmitMethod(symbol);
		else if (strcmp(kind, "constructor") == 0)
			EmitConstructor(symbol);
		else if (strcmp(kind, "function") == 0)
			EmitFunction1(symbol);
	}
		

	NEXT_TOKEN

	// Check if open parenthesis is found
	if(t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	EnterScope(symbol);

	pi = ParamList();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	if(t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeParenExpected, ") expected");
		return pi;
	}

	pi = SubroutineBody();	

	if (pi.er != none)
		return pi;

	ExitScope();

	return pi;
}

ParserInfo ParamList()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	// Find the parent class, for type
	Scope* classScope = FindParentClass();
	//Create this symbol for method
	char* name = classScope->scopeSymbol->name;
	Scope* currentScope = GetCurrentScope();
	Symbol* symbol;

	// If current scope is not a function, then create "this" symbol
	if (strcmp(currentScope->scopeSymbol->kind, "method") == 0)
	{
		if (secondPass == false)
			symbol = CreateSymbolAtCurrentScope("this", name, "argument", pi, 0);
	}

	if (strcmp(currentScope->scopeSymbol->kind, "constructor") == 0)
	{
		if (secondPass == false)
			symbol = CreateSymbolAtCurrentScope("this", name, "THIS", pi, 0);
	}

	PEEK_TOKEN

	// Check if close parenthesis is found
	if(t.tp != RESWORD && t.tp != ID)
		return pi;

	pi = Type();

	if (pi.er != none)
		return pi;

	// Store type of param
	char type[128];
	strcpy(type, t.lx);

	NEXT_TOKEN

	// Check if param name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "parameter name expected");
		return pi;
	}

	// add symbol to LOCAL subroutine scope
	symbol = FindSymbolAtCurrentScope(t.lx);

	if (symbol != NULL && secondPass == false)
	{
		Error(&pi, &t, redecIdentifier, "parameter already exists");
		return pi;
	}

	if (secondPass == false)
		CreateSymbolAtCurrentScope(t.lx, type, "argument", pi, 0);

	PEEK_TOKEN

	// loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip comma
		NEXT_TOKEN

		pi = Type();

		if (pi.er != none)
			return pi;

		// Store type of param
		strcpy(type, pi.tk.lx);

		NEXT_TOKEN

		// Check if param name is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "parameter name expected");
			return pi;
		}

		symbol = FindSymbolAtCurrentScope(t.lx);

		if (symbol != NULL && secondPass == false)
		{
			Error(&pi, &t, redecIdentifier, "parameter already exists");
			return pi;
		}

		if (secondPass == false)
			CreateSymbolAtCurrentScope(t.lx, type, "argument", pi, 0);

		PEEK_TOKEN

		// If next token is not comma, break
		if(t.tp != SYMBOL || strcmp(t.lx, ",") != 0)
			break;
	}

	return pi;
}

ParserInfo SubroutineBody()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if open curly brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

	PEEK_TOKEN

	// loop while the next token is not }
	while(t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		pi = Statement();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
	}

	NEXT_TOKEN

	// Check if close curly brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		Error(&pi, &t, closeBraceExpected, "} expected");
		return pi;
	}

	return pi;
}

ParserInfo Statement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	PEEK_TOKEN

	// Expected var, let, if, while, do, return
	if (t.tp != RESWORD)
	{
		Error(&pi, &t, syntaxError, "var, let, if, while, do or return statement expected");
		return pi;
	}

	// Check if var statement
	if (strcmp(t.lx, "var") == 0)
	{
		pi = VarDeclarStatement();

		if (pi.er != none)
			return pi;
	}
	// Check if let statement
	else if (strcmp(t.lx, "let") == 0)
	{
		pi = LetStatement();

		if (pi.er != none)
			return pi;
	}
	// Check if if statement
	else if (strcmp(t.lx, "if") == 0)
	{
		pi = IfStatement();

		if (pi.er != none)
			return pi;
	}
	// Check if while statement
	else if (strcmp(t.lx, "while") == 0)
	{
		pi = WhileStatement();

		if (pi.er != none)
			return pi;
	}
	// Check if do statement
	else if (strcmp(t.lx, "do") == 0)
	{
		pi = DoStatement();

		if (pi.er != none)
			return pi;
	}
	// Check if return statement
	else if (strcmp(t.lx, "return") == 0)
	{
		pi = ReturnStatement();

		if (pi.er != none)
			return pi;
	}
	else
	{
		Error(&pi, &t, syntaxError, "var, let, if, while, do or return statement expected");
		return pi;
	}

	return pi;
}

ParserInfo VarDeclarStatement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if var keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "var") != 0)
	{
		Error(&pi, &t, syntaxError, "var keyword expected");
		return pi;
	}

	pi = Type();

	if (pi.er != none)
		return pi;

	// Store type of var
	char type[128];
	strcpy(type, pi.tk.lx);

	NEXT_TOKEN

	// Check if var name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "variable name expected");
		return pi;
	}

	if (FindSymbolAtCurrentScope(t.lx) != NULL && secondPass == false)
	{
		Error(&pi, &t, redecIdentifier, "variable already exists");
		return pi;
	}

	if (secondPass == false)
		CreateSymbolAtCurrentScope(t.lx, type, "var", pi, 0);

	PEEK_TOKEN

	// Loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip comma
		NEXT_TOKEN

		// Get next token
		NEXT_TOKEN

		// Check if var name is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "variable name expected");
			return pi;
		}

		if (SearchSymbolFromCurrentScope(t.lx) != NULL && secondPass == false)
		{
			Error(&pi, &t, redecIdentifier, "variable already exists");
			return pi;
		}

		if(secondPass == false)
			CreateSymbolAtCurrentScope(t.lx, type, "var", pi, 0);

		PEEK_TOKEN
	}

	NEXT_TOKEN

	// Check if semicolon is found
	if(t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	return pi;
}

ParserInfo LetStatement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if let keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "let") != 0)
	{
		Error(&pi, &t, syntaxError, "let keyword expected");
		return pi;
	}

	NEXT_TOKEN

	// Check if var name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "variable name expected");
		return pi;
	}

	Symbol* letVar = SearchSymbolFromCurrentScope(t.lx);
	bool letArray = false;

	// Check if symbol is in symbol table
	if(letVar == NULL)
	{
		char errorMsg[128];
		safe_snprintf(errorMsg, sizeof(errorMsg), "variable (%s) not declared", t.lx);

		Error(&pi, &t, undecIdentifier, errorMsg);
		return pi;
	}

	PEEK_TOKEN

	// Check if next token is "[" or "="
	if (t.tp != SYMBOL)
	{
		Error(&pi, &t, equalExpected, "[ or = expected");
		return pi;
	}

	// if next token is [ then check for expression
	if (strcmp(t.lx, "[") == 0)
	{
		letArray = true;
		NEXT_TOKEN

		pi = Expression();

		NEXT_TOKEN

		// Check if next token is ]
		if (t.tp != SYMBOL || strcmp(t.lx, "]") != 0)
		{
			Error(&pi, &t, closeBraceExpected, "] expected");
			return pi;
		}

		if (secondPass)
		{
			EmitPushLocal(letVar->address);
			EmitCode("add\n");
		}
	}

	NEXT_TOKEN

	// Check if next token is =
	if (t.tp != SYMBOL || strcmp(t.lx, "=") != 0)
	{
		Error(&pi, &t, equalExpected, "= expected");
		return pi;
	}

	pi = Expression();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if next token is ;
	if (t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	if(secondPass == true)
	{
		if(letArray)
			EmitLetArray(letVar->address);
		else
			EmitPop(letVar);
	}

	return pi;
}

ParserInfo IfStatement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if if keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "if") != 0)
	{
		Error(&pi, &t, syntaxError, "if keyword expected");
		return pi;
	}

	NEXT_TOKEN

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	pi = Expression();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if next token is )
	if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeParenExpected, ") expected");
		return pi;
	}

	int ifIndex = 0;
	if(secondPass)
		ifIndex = EmitIfStart();

	NEXT_TOKEN

	// Check if next token is {
	if (t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

	// loop while next token is not }
	while (t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		pi = Statement();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
	}

	// Skip }
	NEXT_TOKEN
	
	PEEK_TOKEN

	// Check if next token is else
	if (t.tp == RESWORD && strcmp(t.lx, "else") == 0)
	{
		if(secondPass)
			EmitElseStart(ifIndex);

		// Skip else keyword
		NEXT_TOKEN

		// Get next token
		NEXT_TOKEN

		// Check if next token is {
		if (t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
		{
			Error(&pi, &t, openBraceExpected, "{ expected");
			return pi;
		}

		// loop while next token is not }
		while (t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
		{
			pi = Statement();

			if (pi.er != none)
				return pi;

			PEEK_TOKEN
		}

		// Skip }
		NEXT_TOKEN

		if(secondPass)
			EmitElseEnd(ifIndex);
	}
	else
	{
		if(secondPass)
			EmitIfEnd(ifIndex);
	}

	return pi;
}

ParserInfo WhileStatement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if while keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "while") != 0)
	{
		Error(&pi, &t, syntaxError, "while keyword expected");
		return pi;
	}

	int whileIndex = 0;
	if(secondPass)
		whileIndex = EmitStartWhile1();

	NEXT_TOKEN

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	pi = Expression();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if next token is )
	if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeBraceExpected, ") expected");
		return pi;
	}

	if(secondPass)
		EmitStartWhile2(whileIndex);

	NEXT_TOKEN

	// Check if next token is {
	if (t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

	while (t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		pi = Statement();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
	}

	// Skip }
	NEXT_TOKEN

	if(secondPass)
		EmitEndWhile(whileIndex);

	return pi;
}

ParserInfo DoStatement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if do keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "do") != 0)
	{
		Error(&pi, &t, syntaxError, "do keyword expected");
		return pi;
	}

	pi = SubroutineCall();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if next token is ;
	if (t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	if (secondPass == true)
	{	
		EmitPopTemp(0);
	}
	
	return pi;
}

ParserInfo SubroutineCall()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if next token is identifier
	if (t.tp != ID)
	{
		Error(&pi, &t, idExpected, "identifier expected");
		return pi;
	}

	// Store identifier of subroutine or class
	char identifier[128];
	strcpy(identifier, t.lx);

	PEEK_TOKEN

	Symbol* subroutineSymbol = NULL;
	Symbol* subroutineCaller = NULL;
	Scope* classScope;

	// Check if next token is .
	if (t.tp == SYMBOL && strcmp(t.lx, ".") == 0)
	{
		//If next token is . then the identifier is a class name or variable name that references a class
		char name[128];
		strcpy(name, identifier);
		subroutineCaller = SearchSymbolFromCurrentScope(name);
		
		// If symbol is not found then create a class
		if (subroutineCaller == NULL)
		{
			// Create undecleared class
			classScope = CreateClass(name, name, "NULL", pi);
		}
		else
		{
			classScope = FindClass(subroutineCaller->type);

			if (classScope == NULL)
			{
				Error(&pi, &t, syntaxError, "variable does not reference a class");
				return pi;
			}
		}

		// Skip "." token
		NEXT_TOKEN
		// Get next token
		NEXT_TOKEN

		// Check if next token is identifier
		if (t.tp != ID)
		{
			Error(&pi, &t, idExpected, "identifier expected");
			return pi;
		}

		subroutineSymbol = SearchGlobalSymbol(classScope->scopeSymbol->name, t.lx);

		// Check if subroutine is declared
		if (subroutineSymbol == NULL)
		{
			// Create undecleared subroutine
			CreateSymbolAtScope(classScope, t.lx, "NULL", "NULL", pi, 1);
		}
	}
	else
	{
		// If next token is not . then the identifier is a subroutine name
		subroutineSymbol = SearchSymbolFromCurrentScope(identifier);

		if(subroutineSymbol == NULL)
		{
			classScope = FindParentClass();

			if (classScope == NULL)
			{
				Error(&pi, &t, syntaxError, "Subroutine call outside class");
			}		

			// Create undecleared subroutine
			CreateSymbolAtScope(classScope, identifier, "NULL", "NULL", pi, 1);	
		}	
	}

	NEXT_TOKEN

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}
	
	if(secondPass == true)
		EmitCaller(subroutineCaller);

	pi = ExpressionList();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if next token is )
	if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeBraceExpected, ") expected");
		return pi;
	}

	if(secondPass == true)
		EmitCall1(subroutineSymbol);

	return pi;
}

ParserInfo ExpressionList()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	PEEK_TOKEN

	// Check if next token is ), if yes then there are no expressions
	if (t.tp == SYMBOL && strcmp(t.lx, ")") == 0)
		return pi;

	pi = Expression();

	if (pi.er != none)
		return pi;
	
	PEEK_TOKEN

	// Loop while next token is ,
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip "," token
		NEXT_TOKEN

		pi = Expression();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
	}

	return pi;
}

ParserInfo ReturnStatement()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if return keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "return") != 0)
	{
		Error(&pi, &t, syntaxError, "return keyword expected");
		return pi;
	}

	PEEK_TOKEN

	if (strcmp(t.lx, ";") != 0)
	{
		pi = Expression();

		if (pi.er != none)
			return pi;
	}
	else
	{
		if (secondPass == true)
			EmitPushConstant(0);
	}

	if (secondPass == true)
		EmitReturn();

	NEXT_TOKEN

	// Check if next token is semicolon
	if (t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	return pi;
}

ParserInfo Expression()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	pi = RelationalExpression();

	if (pi.er != none)
		return pi;

	PEEK_TOKEN

	//loop while next token is | or &
	while (t.tp == SYMBOL && (strcmp(t.lx, "|") == 0 || strcmp(t.lx, "&") == 0))
	{
		char op[128];
		strcpy(op, t.lx);
		
		// Skip "|" or "&" token
		NEXT_TOKEN

		pi = RelationalExpression();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN

		if (secondPass == true)
		{
			if(strcmp(op, "|") == 0)
				EmitCode("or\n");
			else
				EmitCode("and\n");
		}
	}

	return pi;
}

ParserInfo RelationalExpression()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	pi = ArithmeticExpression();

	if (pi.er != none)
		return pi;

	PEEK_TOKEN

	//loop while next token is <, >, =
	while (t.tp == SYMBOL && (strcmp(t.lx, "<") == 0 || strcmp(t.lx, ">") == 0 || strcmp(t.lx, "=") == 0))
	{
		char op[128];
		strcpy(op, t.lx);

		// Skip <, > or = token
		NEXT_TOKEN

		pi = ArithmeticExpression();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN

		if (secondPass == true)
		{
			if(strcmp(op, "<") == 0)
				EmitCode("lt\n");
			else if(strcmp(op, ">") == 0)
				EmitCode("gt\n");
			else if(strcmp(op, "=") == 0)
				EmitCode("eq\n");
		}
	}

	return pi;
}

ParserInfo ArithmeticExpression()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	pi = Term();

	if (pi.er != none)
		return pi;

	PEEK_TOKEN

	//loop while next token is + or -
	while (t.tp == SYMBOL && (strcmp(t.lx, "+") == 0 || strcmp(t.lx, "-") == 0))
	{
		char op[128];
		strcpy(op, t.lx);

		// Skip + or - token
		NEXT_TOKEN

		pi = Term();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN

		if (secondPass == true)
		{
			if(strcmp(op, "+") == 0)
				EmitCode("add\n");
			else if(strcmp(op, "-") == 0)
				EmitCode("sub\n");
		}
	}
	
	return pi;
}

ParserInfo Term()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	pi = Factor();

	if (pi.er != none)
		return pi;

	PEEK_TOKEN

	//loop while next token is * or /
	while (t.tp == SYMBOL && (strcmp(t.lx, "*") == 0 || strcmp(t.lx, "/") == 0))
	{
		char op[128];
		strcpy(op, t.lx);

		// Skip * or / token
		NEXT_TOKEN

		pi = Factor();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN

		if (secondPass == true)
		{
			if(strcmp(op, "*") == 0)
				EmitMultiply();
			else if(strcmp(op, "/") == 0)
				EmitDivide();
		}
	}

	return pi;
}

ParserInfo Factor()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	PEEK_TOKEN

	char op[128];

	// next symbol should be - or ~ or empty string
	if (t.tp == SYMBOL)
	{
		if	(strcmp(t.lx, "-") == 0 || strcmp(t.lx, "~") == 0)
		{
			strcpy(op, t.lx);

			// Skip - or ~ token
			NEXT_TOKEN
		}
	}

	pi = Operand();

	if (pi.er != none)
		return pi;

	if (secondPass)
	{
		if(strcmp(op, "-") == 0)
			EmitCode("neg\n");
		else if(strcmp(op, "~") == 0)
			EmitCode("not\n");
	}
	
	return pi;
}

ParserInfo Operand()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	PEEK_TOKEN

	// if int constant
	if (t.tp == INT)
	{
		if(secondPass)
			EmitPushConstant(atoi(t.lx));

		// Skip int constant token
		NEXT_TOKEN
	}
	// if it's identifier
	else if(t.tp == ID)
	{
		pi = OperantIdentifier();

		if (pi.er != none)
			return pi;
	}
	// if next token is (
	else if (t.tp == SYMBOL && strcmp(t.lx, "(") == 0)
	{
		// Skip ( token
		NEXT_TOKEN

		pi = Expression();

		if (pi.er != none)
			return pi;

		NEXT_TOKEN

		// if next token is not )
		if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
		{
			Error(&pi, &t, closeParenExpected, "Close parenthesis expected");
			return pi;
		}
	}
	// if it's string literal
	else if (t.tp == STRING)
	{
		if(secondPass)
			EmitString(t.lx);

		// Skip string literal token
		NEXT_TOKEN
	}
	else if (t.tp == RESWORD && strcmp(t.lx, "null") == 0)
	{
		if(secondPass)
			EmitPushConstant(0);

		NEXT_TOKEN
	}
	else if (t.tp == RESWORD && strcmp(t.lx, "false") == 0)
	{
		if(secondPass)
			EmitPushConstant(0);

		// Skip true, false or null token
		NEXT_TOKEN
	}
	else if (t.tp == RESWORD && strcmp(t.lx, "true") == 0)
	{
		if(secondPass)
		{
			EmitPushConstant(0);
			EmitCode("not\n");
		}

		NEXT_TOKEN
	}
	// if it's this
	else if(t.tp == RESWORD && strcmp(t.lx, "this") == 0)
	{
		// Skip "this" token
		NEXT_TOKEN

		Symbol* s = SearchSymbolFromCurrentScope(t.lx);

		if (s == NULL)
		{
			Error(&pi, &t, undecIdentifier, "Identifier not declared");
			return pi;
		}

		if(secondPass)
			EmitCode("push pointer 0\n");
	}
	else
	{
		Error(&pi, &t, syntaxError, "Invalid operand");
		return pi;
	}

	return pi;
}

ParserInfo OperantIdentifier()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	NEXT_TOKEN

	// Check if token is identifier
	if (t.tp != ID)
	{
		Error(&pi, &t, idExpected, "Invalid identifier");
		return pi;
	}

	// Store identifier of subroutine or class
	char identifier[128];
	strcpy(identifier, t.lx);

	PEEK_TOKEN

	Symbol* idSymbol = NULL;
	Symbol* caller = NULL;
	Scope* classScope = NULL;

	//if nex token is "." then next token should be identifier
	if (t.tp == SYMBOL && strcmp(t.lx, ".") == 0)
	{
		//If next token is . then the identifier is a class name or variable name that references a class
		char name[128];
		strcpy(name, identifier);
		caller = SearchSymbolFromCurrentScope(name);
		
		// If symbol is not found then create a class
		if (caller == NULL)
		{
			// Create undecleared class
			classScope = CreateClass(name, name, "NULL", pi);
		}
		else
		{
			classScope = FindClass(caller->type);

			if (classScope == NULL)
			{
				Error(&pi, &t, syntaxError, "variable does not reference a class");
				return pi;
			}
		}

		// Skip . token
		NEXT_TOKEN

		NEXT_TOKEN

		// Check if token is identifier
		if (t.tp != ID)
		{
			Error(&pi, &t, idExpected, "Invalid identifier");
			return pi;
		}

		// Check if subroutine is declared
		idSymbol = SearchGlobalSymbol(classScope->scopeSymbol->name, t.lx);

		if (idSymbol == NULL)
		{
			// Create undecleared subroutine
			CreateSymbolAtScope(classScope, t.lx, "NULL", "NULL", pi, 1);
		}

		// Peek to prepare for next iteration
		PEEK_TOKEN
	}
	else
	{
		// If next token is not . then the identifier is a subroutine name
		idSymbol = SearchSymbolFromCurrentScope(identifier);

		// Check if subroutine is declared
		if (idSymbol == NULL)
		{
			classScope = FindParentClass();

			if (classScope == NULL)
			{
				Error(&pi, &t, syntaxError, "Subroutine outside class");
				return pi;
			}

			// Create undecleared subroutine
			CreateSymbolAtScope(classScope, identifier, "NULL", "NULL", pi, 1);
		}
	}

	//if next token is [
	if (t.tp == SYMBOL && strcmp(t.lx, "[") == 0)
	{
		// Skip ( token
		NEXT_TOKEN

		pi = Expression();

		if (pi.er != none)
			return pi;

		NEXT_TOKEN

		// Check if token is ]
		if (t.tp != SYMBOL || strcmp(t.lx, "]") != 0)
		{
			Error(&pi, &t, closeBracketExpected, "Close bracket expected");
			return pi;
		}

		if(secondPass)
		{
			EmitAccessArray(idSymbol->address);
		}
	}
	// if next token is (
	else if(t.tp == SYMBOL && strcmp(t.lx, "(") == 0)
	{
		// Skip ( token
		NEXT_TOKEN

		if(secondPass)
			EmitCaller(caller);

		pi = ExpressionList();

		if (pi.er != none)
			return pi;

		NEXT_TOKEN

		// Check if token is )
		if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
		{
			Error(&pi, &t, closeParenExpected, "Close parenthesis expected");
			return pi;
		}

		if(secondPass)
			EmitCall1(idSymbol);
	}
	else
	{
		if (secondPass)
			EmitPush(idSymbol);
	}
	
	return pi;
}

int StopParser ()
{
	ResetCurrentScope();
	return 1;
}
