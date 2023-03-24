#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

void Error(ParserInfo* parserInfo, Token* token, SyntaxErrors syntaxError, char* errorMessage);
void PrintError(char* errorMessage, ParserInfo* parserInfo);
Token GetNextTokenWithErrorCheck(ParserInfo *pi);
Token PeekNextTokenWithErrorCheck(ParserInfo *pi);
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

// you can declare prototypes of parser functions below
void Error(ParserInfo* parserInfo, Token* token, SyntaxErrors syntaxError, char* errorMessage)
{
	parserInfo->er = syntaxError;
	parserInfo->tk = *token;
	PrintError(errorMessage, parserInfo);
}

void TokenError(ParserInfo* parserInfo, Token* token)
{
	parserInfo->er = lexerErr;
	parserInfo->tk = *token;
	printf("%s at line %d in file %s.", token->lx, token->ln, token->fl);
}

void PrintError(char* errorMessage, ParserInfo* parserInfo)
{
	Token token = parserInfo->tk;
	printf("Error: %s at line %d in file %s. Error at %s token.\n", errorMessage, token.ln, token.fl, token.lx);
}

Token GetNextTokenWithErrorCheck(ParserInfo *pi)
{
    Token t = GetNextToken();

    if (t.tp == ERR)
    {
        TokenError(pi, &t);
    }

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
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;
	
	// Check if class keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "class") != 0)
	{
		Error(&pi, &t, classExpected, "class keyword expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if class name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "class name expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if open brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	// Can have many member declarations
	// loop while t is not }
	while (t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		pi = MemberDeclar();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}
	
	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//Cheack if close brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		Error(&pi, &t, closeBraceExpected, "} expected");
		return pi;
	}

	return pi;
}

ParserInfo MemberDeclar()
{
	ParserInfo pi;
	pi.er = none;
	// Peek token to decide to go to class var declaration ot subroutine declaration
	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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
	ParserInfo pi;
	pi.er = none;

	// Can have many class var declarations
	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if static or field is found
	if(t.tp != RESWORD || ((strcmp(t.lx, "static") != 0 && strcmp(t.lx, "field") != 0)))
	{
		Error(&pi, &t, classVarErr, "class variable declaration must begin with static or field keyword");
		return pi;
	}

	pi = Type();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if identifier is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "class variable name expected");
		return pi;
	}

	// Can have many , following with identifier
	t = GetNextTokenWithErrorCheck(&pi);

	// loop while next token is comma
	while (t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if identifier is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "class variable name expected");
			return pi;
		}

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
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
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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

	return pi;
}

ParserInfo SubroutineDeclar()
{
	ParserInfo pi;
	pi.er = none;
	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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

	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if void or type is found
	if(t.tp != RESWORD || strcmp(t.lx, "void") != 0)
	{
		pi = Type();

		if (pi.er != none)
			return pi;
	}
	else if(t.tp == RESWORD && strcmp(t.lx, "void") == 0)
	{
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}
	else
	{
		Error(&pi, &t, subroutineDeclarErr, "subroutine declaration must begin with constructor, function or method keyword");
		return pi;
	}
	
	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if subroutine name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "subroutine name expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if open parenthesis is found
	if(t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	pi = ParamList();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if(t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeParenExpected, ") expected");
		return pi;
	}

	pi = SubroutineBody();	

	if (pi.er != none)
		return pi;

	return pi;
}

ParserInfo ParamList()
{
	ParserInfo pi;
	pi.er = none;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if close parenthesis is found
	if(t.tp != RESWORD && t.tp != ID)
		return pi;

	pi = Type();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if param name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "parameter name expected");
		return pi;
	}

	// Can have many , following with type and identifier
	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip comma
		t = GetNextTokenWithErrorCheck(&pi);

		pi = Type();

		if (pi.er != none)
			return pi;

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if param name is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "parameter name expected");
			// Print error param name not found
		}

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// If next token is not comma, break
		if(t.tp != SYMBOL || strcmp(t.lx, ",") != 0)
			break;
	}

	return pi;
}

ParserInfo SubroutineBody()
{
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if open curly brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	if (pi.er != none)
		return pi;

	// loop while the next token is not }
	while(t.tp != SYMBOL || strcmp(t.lx, "}") != 0)
	{
		pi = Statement();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (pi.er != none)
			return pi;
	}

	// Get the close curly brace
	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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
	ParserInfo pi;
	pi.er = none;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if var keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "var") != 0)
	{
		Error(&pi, &t, syntaxError, "var keyword expected");
		return pi;
	}

	pi = Type();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if var name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "variable name expected");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip comma
		t = GetNextTokenWithErrorCheck(&pi);

		// Get next token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if var name is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "variable name expected");
			return pi;
		}

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if let keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "let") != 0)
	{
		Error(&pi, &t, syntaxError, "let keyword expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if var name is found

	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "variable name expected");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	// Check if next token is "[" or "="
	if (t.tp != SYMBOL)
	{
		Error(&pi, &t, equalExpected, "[ or = expected");
		return pi;
	}

	// if next token is [ then check for expression
	if (strcmp(t.lx, "[") == 0)
	{
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = Expression();

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if next token is ]
		if (t.tp != SYMBOL || strcmp(t.lx, "]") != 0)
		{
			Error(&pi, &t, closeBraceExpected, "] expected");
			return pi;
		}
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is =
	if (t.tp != SYMBOL || strcmp(t.lx, "=") != 0)
	{
		Error(&pi, &t, equalExpected, "= expected");
		return pi;
	}

	pi = Expression();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is ;
	if (t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	return pi;
}

ParserInfo IfStatement()
{
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if if keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "if") != 0)
	{
		Error(&pi, &t, syntaxError, "if keyword expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	pi = Expression();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is )

	if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeParenExpected, ") expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	// Skip }
	t = GetNextTokenWithErrorCheck(&pi);
	
	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is else
	if (t.tp == RESWORD && strcmp(t.lx, "else") == 0)
	{
		// Skip else keyword
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Get next token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

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

			t = PeekNextTokenWithErrorCheck(&pi);

			if (t.tp == ERR)
				return pi;
		}

		// Skip }
		t = GetNextTokenWithErrorCheck(&pi);
	}

	return pi;
}

ParserInfo WhileStatement()
{
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if while keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "while") != 0)
	{
		Error(&pi, &t, syntaxError, "while keyword expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	pi = Expression();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is )
	if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeBraceExpected, ") expected");
		return pi;
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

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

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	// Skip }
	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	return pi;
}

ParserInfo DoStatement()
{
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if do keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "do") != 0)
	{
		Error(&pi, &t, syntaxError, "do keyword expected");
		return pi;
	}

	pi = SubroutineCall();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is ;
	if (t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	return pi;
}

ParserInfo SubroutineCall()
{
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is identifier
	if (t.tp != ID)
	{
		Error(&pi, &t, idExpected, "identifier expected");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is .
	if (t.tp == SYMBOL && strcmp(t.lx, ".") == 0)
	{
		// Skip "." token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if next token is identifier
		if (t.tp != ID)
		{
			Error(&pi, &t, idExpected, "identifier expected");
			return pi;
		}
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

	pi = ExpressionList();

	if (pi.er != none)
		return pi;

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is )
	if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
	{
		Error(&pi, &t, closeBraceExpected, ") expected");
		return pi;
	}

	return pi;
}

ParserInfo ExpressionList()
{
	ParserInfo pi;
	pi.er = none;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is ), if yes then there are no expressions
	if (t.tp == SYMBOL && strcmp(t.lx, ")") == 0)
		return pi;

	pi = Expression();

	if (pi.er != none)
		return pi;
	
	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Loop while next token is ,
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip "," token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = Expression();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	return pi;
}

ParserInfo ReturnStatement()
{
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if return keyword is found
	if(t.tp != RESWORD || strcmp(t.lx, "return") != 0)
	{
		Error(&pi, &t, syntaxError, "return keyword expected");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//Cheack if next token is -, ~ or () and needs expression
	if (t.tp == SYMBOL)
	{
		if (strcmp(t.lx, "-") == 0 || strcmp(t.lx, "~") == 0 || strcmp(t.lx, "(") == 0)
		{
			pi = Expression();

			if (pi.er != none)
				return pi;
		}
	}

	// Check if next token is int, id or string and needs expression
	if (t.tp == INT || t.tp == ID || t.tp == STRING)
	{
		pi = Expression();

		if (pi.er != none)
			return pi;
	}

	// Check if next token is true, false, null or this and needs expression
	if (t.tp == RESWORD)
	{
		if (strcmp(t.lx, "true") == 0 || strcmp(t.lx, "false") == 0 || strcmp(t.lx, "null") == 0 || strcmp(t.lx, "this") == 0)
		{
			pi = Expression();

			if (pi.er != none)
				return pi;
		}
	}

	t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if next token is semicolon
	if (t.tp != SYMBOL || strcmp(t.lx, ";") != 0)
	{
		// Need to go back one line as error is on the previous line
		t.ln--;
		Error(&pi, &t, semicolonExpected, "; expected");
		return pi;
	}

	return pi;
}

ParserInfo Expression()
{
	ParserInfo pi;
	pi.er = none;

	pi = RelationalExpression();

	if (pi.er != none)
		return pi;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//loop while next token is | or &
	while (t.tp == SYMBOL && (strcmp(t.lx, "|") == 0 || strcmp(t.lx, "&") == 0))
	{
		// Skip "|" or "&" token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = RelationalExpression();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	return pi;
}

ParserInfo RelationalExpression()
{
	ParserInfo pi;
	pi.er = none;

	pi = ArithmeticExpression();

	if (pi.er != none)
		return pi;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//loop while next token is <, >, =
	while (t.tp == SYMBOL && (strcmp(t.lx, "<") == 0 || strcmp(t.lx, ">") == 0 || strcmp(t.lx, "=") == 0))
	{
		// Skip <, > or = token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = ArithmeticExpression();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	return pi;
}

ParserInfo ArithmeticExpression()
{
	ParserInfo pi;
	pi.er = none;

	pi = Term();

	if (pi.er != none)
		return pi;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//loop while next token is + or -
	while (t.tp == SYMBOL && (strcmp(t.lx, "+") == 0 || strcmp(t.lx, "-") == 0))
	{
		// Skip + or - token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = Term();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}
	
	return pi;
}

ParserInfo Term()
{
	ParserInfo pi;
	pi.er = none;

	pi = Factor();

	if (pi.er != none)
		return pi;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//loop while next token is * or /
	while (t.tp == SYMBOL && (strcmp(t.lx, "*") == 0 || strcmp(t.lx, "/") == 0))
	{
		// Skip * or / token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = Factor();

		if (pi.er != none)
			return pi;

		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	return pi;
}

ParserInfo Factor()
{
	ParserInfo pi;
	pi.er = none;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// next symbol should be - or ~ or empty string
	if (t.tp == SYMBOL)
	{
		if	(strcmp(t.lx, "-") == 0 || strcmp(t.lx, "~") == 0)
		{
			// Skip - or ~ token
			t = GetNextTokenWithErrorCheck(&pi);

			if (t.tp == ERR)
				return pi;
		}
	}

	pi = Operand();

	if (pi.er != none)
		return pi;

	return pi;
}

ParserInfo Operand()
{
	ParserInfo pi;
	pi.er = none;

	Token t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// if int constant
	if (t.tp == INT)
	{
		// Skip int constant token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
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
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = Expression();

		if (pi.er != none)
			return pi;

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

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
		// Skip string literal token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}
	//if it's true or false
	else if (t.tp == RESWORD && (strcmp(t.lx, "true") == 0 || strcmp(t.lx, "false") == 0))
	{
		// Skip true or false token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}
	// if it's null or this
	else if(t.tp == RESWORD && (strcmp(t.lx, "null") == 0 || strcmp(t.lx, "this") == 0))
	{
		// Skip null or this token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
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
	ParserInfo pi;
	pi.er = none;

	Token t = GetNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	// Check if token is identifier
	if (t.tp != ID)
	{
		Error(&pi, &t, idExpected, "Invalid identifier");
		return pi;
	}

	t = PeekNextTokenWithErrorCheck(&pi);

	if (t.tp == ERR)
		return pi;

	//if nex token is "." then next token should be identifier
	if (t.tp == SYMBOL && strcmp(t.lx, ".") == 0)
	{
		// Skip . token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if token is identifier
		if (t.tp != ID)
		{
			Error(&pi, &t, idExpected, "Invalid identifier");
			return pi;
		}

		// Peek to prepare for next iteration
		t = PeekNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;
	}

	//if next token is [
	if (t.tp == SYMBOL && strcmp(t.lx, "[") == 0)
	{
		// Skip ( token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = Expression();

		if (pi.er != none)
			return pi;

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if token is ]
		if (t.tp != SYMBOL || strcmp(t.lx, "]") != 0)
		{
			Error(&pi, &t, closeBracketExpected, "Close bracket expected");
			return pi;
		}
	}
	// if next token is (
	else if(t.tp == SYMBOL && strcmp(t.lx, "(") == 0)
	{
		// Skip ( token
		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		pi = ExpressionList();

		if (pi.er != none)
			return pi;

		t = GetNextTokenWithErrorCheck(&pi);

		if (t.tp == ERR)
			return pi;

		// Check if token is )
		if (t.tp != SYMBOL || strcmp(t.lx, ")") != 0)
		{
			Error(&pi, &t, closeParenExpected, "Close parenthesis expected");
			return pi;
		}
	}
	
	return pi;
}

int StopParser ()
{
	return 1;
}

#ifndef TEST_PARSER
int main ()
{
	//Init parser
	InitParser("semicolonExpected.jack");

	//Start parsing
	Parse();

	//Stop parser
	StopParser();

	return 1;
}
#endif
