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

#define NEXT_TOKEN t = GetNextTokenWithErrorCheck(&pi); if(t.tp == ERR){TokenError(&pi, &t); return pi;}
#define PEEK_TOKEN t = PeekNextTokenWithErrorCheck(&pi); if(t.tp == ERR){TokenError(&pi, &t); return pi;}

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

	NEXT_TOKEN

	// Check if open brace is found
	if(t.tp != SYMBOL || strcmp(t.lx, "{") != 0)
	{
		Error(&pi, &t, openBraceExpected, "{ expected");
		return pi;
	}

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

	// Check if static or field is found
	if(t.tp != RESWORD || ((strcmp(t.lx, "static") != 0 && strcmp(t.lx, "field") != 0)))
	{
		Error(&pi, &t, classVarErr, "class variable declaration must begin with static or field keyword");
		return pi;
	}

	pi = Type();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if identifier is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "class variable name expected");
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
	
	NEXT_TOKEN

	// Check if subroutine name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "subroutine name expected");
		return pi;
	}

	NEXT_TOKEN

	// Check if open parenthesis is found
	if(t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

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

	return pi;
}

ParserInfo ParamList()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	PEEK_TOKEN

	// Check if close parenthesis is found
	if(t.tp != RESWORD && t.tp != ID)
		return pi;

	pi = Type();

	if (pi.er != none)
		return pi;

	NEXT_TOKEN

	// Check if param name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "parameter name expected");
		return pi;
	}

	PEEK_TOKEN

	// loop while next token is comma
	while(t.tp == SYMBOL && strcmp(t.lx, ",") == 0)
	{
		// Skip comma
		NEXT_TOKEN

		pi = Type();

		if (pi.er != none)
			return pi;

		NEXT_TOKEN

		// Check if param name is found
		if(t.tp != ID)
		{
			Error(&pi, &t, idExpected, "parameter name expected");
			return pi;
		}

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

	NEXT_TOKEN

	// Check if var name is found
	if(t.tp != ID)
	{
		Error(&pi, &t, idExpected, "variable name expected");
		return pi;
	}

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
		NEXT_TOKEN

		pi = Expression();

		NEXT_TOKEN

		// Check if next token is ]
		if (t.tp != SYMBOL || strcmp(t.lx, "]") != 0)
		{
			Error(&pi, &t, closeBraceExpected, "] expected");
			return pi;
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

	PEEK_TOKEN

	// Check if next token is .
	if (t.tp == SYMBOL && strcmp(t.lx, ".") == 0)
	{
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
	}

	NEXT_TOKEN

	// Check if next token is (
	if (t.tp != SYMBOL || strcmp(t.lx, "(") != 0)
	{
		Error(&pi, &t, openParenExpected, "( expected");
		return pi;
	}

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
		// Skip "|" or "&" token
		NEXT_TOKEN

		pi = RelationalExpression();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
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
		// Skip <, > or = token
		NEXT_TOKEN

		pi = ArithmeticExpression();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
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
		// Skip + or - token
		NEXT_TOKEN

		pi = Term();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
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
		// Skip * or / token
		NEXT_TOKEN

		pi = Factor();

		if (pi.er != none)
			return pi;

		PEEK_TOKEN
	}

	return pi;
}

ParserInfo Factor()
{
	Token t;
	ParserInfo pi;
	pi.er = none;

	PEEK_TOKEN

	// next symbol should be - or ~ or empty string
	if (t.tp == SYMBOL)
	{
		if	(strcmp(t.lx, "-") == 0 || strcmp(t.lx, "~") == 0)
		{
			// Skip - or ~ token
			NEXT_TOKEN
		}
	}

	pi = Operand();

	if (pi.er != none)
		return pi;

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
		// Skip string literal token
		NEXT_TOKEN
	}
	//if it's true, false or null
	else if (t.tp == RESWORD && (strcmp(t.lx, "true") == 0 || strcmp(t.lx, "false") == 0 || strcmp(t.lx, "null") == 0))
	{
		// Skip true, false or null token
		NEXT_TOKEN
	}
	// if it's null or this
	else if(t.tp == RESWORD && strcmp(t.lx, "this") == 0)
	{
		// Skip null or this token
		NEXT_TOKEN
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

	PEEK_TOKEN

	//if nex token is "." then next token should be identifier
	if (t.tp == SYMBOL && strcmp(t.lx, ".") == 0)
	{
		// Skip . token
		NEXT_TOKEN

		NEXT_TOKEN

		// Check if token is identifier
		if (t.tp != ID)
		{
			Error(&pi, &t, idExpected, "Invalid identifier");
			return pi;
		}

		// Peek to prepare for next iteration
		PEEK_TOKEN
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
	}
	// if next token is (
	else if(t.tp == SYMBOL && strcmp(t.lx, "(") == 0)
	{
		// Skip ( token
		NEXT_TOKEN

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
	InitParser("Output.jack");

	//Start parsing
	Parse();

	//Stop parser
	StopParser();

	return 1;
}
#endif
