#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"


// you can declare prototypes of parser functions below




int InitParser (char* file_name)
{
	InitLexer(file_name);
	return 1;
}

ParserInfo Parse ()
{
	ParserInfo pi;

	// implement the function


	pi.er = none;
	return pi;
}

void ClassDeclar()
{
	Token t = GetNextToken();
	
	if(t.tp != RESWORD || strcmp(t.lx, "class") != 0)
	{
		// Print error class resword not found
	}

	t = GetNextToken();

	if(t.tp != ID)
	{
		// Print error class name not found
	}

	t = GetNextToken();

	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		// Print error class body not found
	}

	// Can have many member declarations
	MemberDeclar();
}

void MemberDeclar()
{
	// Peek token to decide to go to class var declaration ot subroutine declaration
	Token t = PeekNextToken();

	if(t.tp != RESWORD)
	{
		// Print error member declaration not found
	}

	if(strcmp(t.lx, "constructor") == 0 || strcmp(t.lx, "function") == 0 || strcmp(t.lx, "method") == 0)
	{
		SubroutineDeclar();
	}
	else if(strcmp(t.lx, "static") == 0 || strcmp(t.lx, "field") == 0)
	{
		ClassVarDeclar();
	}
	else
	{
		// Print error member declaration not found
	}
}

void SubroutineDeclar()
{
	Token t = GetNextToken();

	if(t.tp != RESWORD)
	{
		// Print error subroutine declaration not found
	}

	if(strcmp(t.lx, "constructor") != 0 && strcmp(t.lx, "function") != 0 && strcmp(t.lx, "method") != 0)
	{
		// Print error invalid subroutine declaration
	}

	t = PeekNextToken();

	if(t.tp != RESWORD || strcmp(t.lx, "void") != 0)
	{
		Type();
	}

	t = GetNextToken();

	if(t.tp != ID)
	{
		// Print error subroutine name not found
	}

	t = GetNextToken();

	if(t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		// Print error ( not found
	}

	ParamList();

	t = GetNextToken();

	if(t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		// Print error ) not found
	}

	SubroutineBody();	
}

void ParamList()
{
	type();

	Token t = GetNextToken();

	if(t.tp != ID)
	{
		// Function does not have parameters
		return;
	}

	// Can have many , following with type and identifier
	t = GetNextToken();

	// loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		type();

		t = GetNextToken();

		if(t.tp != ID)
		{
			// Print error param name not found
		}

		t = PeekNextToken();

		if(t.tp != SYMBOL || strcmp(t.lx, ",") != 0)
		{
			// End of parameters
			return;
		}

		t = GetNextToken();
	}
}

void SubroutineBody()
{
	Token t = GetNextToken();

	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		// Print error { not found
	}

	//Can have many statements
	Statement();
}

void ClassVarDeclar()
{
	// Can have many class var declarations
	Token t = GetNextToken();

	if(t.tp != RESWORD || ((strcmp(t.lx, "static") != 0 && strcmp(t.lx, "field") != 0)))
	{
		// Print error class var declaration not found
	}

	Type();

	t = GetNextToken();

	if(t.tp != ID)
	{
		// Print error class var name not found
	}

	// Can have many , following with identifier
	t = GetNextToken();

	// loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		t = GetNextToken();

		if(t.tp != ID)
		{
			// Print error class var name not found
		}

		t = GetNextToken();
	}

	if(t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		// Print error ; missing
	}
}

void Type()
{
	Token t = GetNextToken();

	// in lextures it says that there should be ID however it does not make sense
	if(t.tp != RESWORD)
	{
		// Print error type is not resword or id
	}

	if(strcmp(t.lx, "int") != 0 && strcmp(t.lx, "char") != 0 && strcmp(t.lx, "boolean") != 0)
	{
		// Print error invalid type
	}

	return;
}

int StopParser ()
{
	return 1;
}

#ifndef TEST_PARSER
int main ()
{

	return 1;
}
#endif
