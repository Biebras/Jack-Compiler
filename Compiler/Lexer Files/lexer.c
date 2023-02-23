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


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
FILE* file;


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

    return 1;
}

/// @brief Ignores comments and white spaces
/// @param c starting char in file 
/// @return On ignore success returns 1, on error returns 0
int IgnoreSpacesAndComments()
{
    char c = getc(file);

    // Check for EOF
    if (c == EOF)
    {
        return 1;
    }

    while (isblank(c) || c == '\r' || c == '\n' || c == '/')
    {
        if(isblank(c) || c == '\n')
        {
            //ignore spaces
            while (isblank(c) || c == '\n')
            {
                c = getc(file);
            }
        }

        //ignore comments
        if (c == '/')
        {
            c = getc(file);

            //single line comment
            if (c == '/')
            {
                while (c != '\n' && c != EOF)
                {
                    c = getc(file);
                }
            }
            // multi line comment
            else if (c == '*')
            {
                while (1)
                {
                    c = getc(file);

                    if (c == EOF)
                        return 0;
                    
                    // might be end of comment
                    if (c == '*')
                    {
                        c = getc(file);

                        if (c == EOF)
                            return 0;

                        // end of comment
                        if(c == '/')
                            break;
                    }
                }
            }
        }
        else if (c == '\r')
        {
            c = getc(file);
        }
    }

    //undo last character as we needed to peek befor deciding the end of comment
    ungetc(c, file);

    return 1;
}

// Get the next token from the source file
Token GetNextToken ()
{
    //Token init
    Token t;
    t.tp = ERR;

    string result = IgnoreSpacesAndComments();

    if (result == 0)
    {
        t.ec = (int)EofInCom;
        return t;
    }

    char tmp = 
    char c = getc(file);




    return t;
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

// do not remove the next line
#ifndef TEST
int main ()
{
	InitLexer("Ball.jack");

    int result = IgnoreSpacesAndComments();

    if (result == 0)
    {
        printf("Ignore error\n");
        return 0;
    }

    char c = getc(file);
    printf("New start line: %c\n", c);

	return 1;
}
// do not remove the next line
#endif
