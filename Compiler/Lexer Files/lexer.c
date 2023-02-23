/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include <unistd.h>


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
FILE* file;
int lineNumber = 1;
char fileName[32]; 

// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer (char* file_name)
{
    file = fopen(file_name, "r");
    
    if (file == 0)
    {
        printf("Error: can't open file\n");
        return 0;
    }

    //only allow with .jack extension
    if (strstr(file_name, ".jack") == NULL)
    {
        printf("Error: file must have .jack extension\n");
        return 0;
    }

    strcpy(fileName, file_name);
    return 1;
}

/// @brief Get's next character in file. If next char is line break, iterate lline number
/// @return return next character
char GetChar()
{
    int ASCII = fgetc(file);
    int c = (char)ASCII;

    if (c == '\n')
        lineNumber++;

    return c;
}

//Peek character in file
char PeekChar()
{
    int ASCII = fgetc(file);
    int c = (char)ASCII;

    ungetc(c, file);

    return c;
}

/// @brief Ignores comments and white spaces
/// @param c starting char in file 
/// @return On ignore success returns 1, on error returns 0
int IgnoreSpacesAndComments()
{
    char c = GetChar();

    // ignore white spaces and carriage returns and line breaks
    while (isspace(c) || c == '\r' || c == '\n')
    {
        c = GetChar();
    }

    // Make sure that the comment is not a devide operator
    if (c == '/')
    {
        char peek = PeekChar();

        if (peek != '/' && peek != '*')
        {
            ungetc(c, file);
            return 1;
        }
    }

    // ignore line comments
    if (c == '/' && PeekChar() == '/')
    {
        c = GetChar();

        while (c != '\n')
        {
            c = GetChar();
        }

        return IgnoreSpacesAndComments();
    }

    // ignore block comments
    if (c == '*')
    {
        while (1)
        {
            c = GetChar();

            if (c == EOF)
                return 0;

            if (c == '*')
            {
                c = GetChar();

                if (c == EOF)
                    return 0;

                if (c == '/')
                {
                    return IgnoreSpacesAndComments();
                }
            }
        }
    }

    // unget the last character
    ungetc(c, file);
    
    return 1;
}

Token EndOfFile(Token* token)
{
    strcpy(token->lx, "End of File");
    token->tp = EOFile;
    token->ln = lineNumber;
    return *token;
}

// Get the next token from the source file
Token GetNextToken ()
{
    //Token init
    Token token;
    token.tp = ERR;
    //assign file name to token
    strcpy(token.fl, fileName);

    int result = IgnoreSpacesAndComments();

    if (result == 0)
    {
        strcpy(token.lx, "Error: unexpected eof in comment");
        token.tp = ERR;
        token.ln = lineNumber;
        token.ec = (int)EofInCom;
        return token;
    }

    char tmp[128] = "";
    char c = GetChar();
    int charIndex = 0;

    if (c == EOF)
        return EndOfFile(&token);

    //character is number
    if (isalpha(c))
    {
        while (isalpha(c))
        {
            tmp[charIndex] = c;
            charIndex++;
            c = GetChar();
        }
        
        ungetc(c, file);
        tmp[charIndex] = '\0';
        //copy string
        strcpy(token.lx, tmp);
        //assign line number
        token.ln = lineNumber;

        int keywordCount = sizeof(keywords) / sizeof(keywords[0]);

        //Handle keywords
        for (int i = 0; i < keywordCount; i++)
        {
            if(strcmp(tmp, keywords[i]) == 0)
            {
                token.tp = RESWORD;                
                return token;
            }
        }

        token.tp = ID;
        return token;
    }
    // Handle strings
    else if (c == '"')
    {
        c = GetChar();

        while (c != '"')
        {
            // Check for EOF
            if (c == EOF)
            {
                strcpy(token.lx, "Error: unexpected eof in string constant");
                token.tp = ERR;
                token.ln = lineNumber;
                token.ec = (int)EofInStr;
                return token;
            }
            
            tmp[charIndex] = c;
            charIndex++;
            c = GetChar();
        }

        tmp[charIndex] = '\0';
        //copy string
        strcpy(token.lx, tmp);
        //assign line number
        token.ln = lineNumber;
        //asign token type
        token.tp = STRING;
        
        return token;
    }
    else if(isnumber(c))
    {
        while (isnumber(c))
        {
            tmp[charIndex] = c;
            charIndex++;
            c = PeekChar();
        }

        tmp[charIndex] = '\0';
        //copy string
        strcpy(token.lx, tmp);
        //assign line number
        token.ln = lineNumber;
        //asign token type
        token.tp = INT;

        return token;
    }
    else //must be a symbol
    {
        int symbolCount = sizeof(legalSymbols) / sizeof(legalSymbols[0]);

        //Handle symbols
        for (int i = 0; i < symbolCount; i++)
        {
            if(c == legalSymbols[i])
            {
                token.tp = SYMBOL;
                token.lx[0] = c;
                token.lx[1] = '\0';
                token.ln = lineNumber;
                return token;
            }
        }

        token.ec = (int)IllSym;
        return token;
    }

    return token;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
    //Token init
    Token t;
    t.tp = ERR;

    return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{
	return 0;
}

// Return the string representation of the token type
char* GetTokenName (TokenType tp)
{
    switch (tp)
    {
        case RESWORD:
            return "RESWORD";
        case ID:
            return "ID";
        case INT:
            return "INT";
        case SYMBOL:
            return "SYMBOL";
        case STRING:
            return "STRING";
        case EOFile:
            return "EOFile";
        case ERR:
            return "ERR";
        default:
            return "ERR";
    }
}

// do not remove the next line
#ifndef TEST
int main ()
{
    //get current directory
	int result = InitLexer("Ball.jack");

    if (result == 0)
        return 0;

    Token token = GetNextToken();
    printf("< %s, %d, %s, %s >\n", token.fl, token.ln, token.lx, GetTokenName(token.tp));

    while (token.tp != EOFile && token.tp != ERR)
    {
        token = GetNextToken();
        printf("< %s, %d, %s, %s >\n", token.fl, token.ln, token.lx, GetTokenName(token.tp));
    }

	return 1;
}
// do not remove the next line
#endif
