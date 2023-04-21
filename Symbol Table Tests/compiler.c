/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "compiler.h"

/// @brief Returns all files with .jack extension in the given directory. (From bing chat)
/// @param dirPath Path to the directory.
/// @param numFiles Number of files found.
/// @return Array of file names.
char** getJackFiles(char* dirPath, int* numFiles) 
{
    DIR* dir = opendir(dirPath);

    if (dir == NULL) 
	{
        printf("Could not open directory %s\n", dirPath);
        return NULL;
    }

    struct dirent* entry;
    int count = 0;
    char** files = NULL;

    while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_type == DT_REG) 
		{
            char* ext = strrchr(entry->d_name, '.');

            if (ext && strcmp(ext, ".jack") == 0) 
			{
                files = realloc(files, sizeof(char*) * (count + 1));
                files[count] = malloc(strlen(entry->d_name) + 1);
                strcpy(files[count], entry->d_name);
                count++;
            }
        }
    }

    closedir(dir);
    *numFiles = count;
    return files;
}

int InitCompiler ()
{
	return 1;
}

ParserInfo compile (char* dir_name)
{
	ParserInfo p;
	p.er = none;

	int numFiles;
	char** buildInFiles = getJackFiles(".", &numFiles);

	InitSymbolTable();

	for (int i = 0; i < numFiles; i++)
	{
		InitParser(buildInFiles[i]);
		ParserInfo pi = Parse();
		StopParser();

		if (pi.er != none)
		{
			return pi;
		}
	}

	char** directoryFiles = getJackFiles(dir_name, &numFiles);

	for (int i = 0; i < numFiles; i++)
	{
		char filePath[1024];
		snprintf(filePath, sizeof(filePath), "%s/%s", dir_name, directoryFiles[i]);
		
		InitParser(filePath);
		ParserInfo pi = Parse();
		StopParser();
		
		if (pi.er != none)
		{
			return pi;
		}
	}

	Symbol* undeclared = SearchForUndeclaredSymbol();

	if (undeclared != NULL)
	{
		Token token = undeclared->pi.tk;
		char* errorMessage = "Undeclared symbol";
		printf("Error: %s. Accured at line %d near %s token in file %s.\n", errorMessage, token.ln, token.lx, token.fl);
		return undeclared->pi;
	}

	return p;
}

int StopCompiler ()
{
	//PrintSymbolTable();
	FreeSymbolTable();
	return 1;
}


#ifndef TEST_COMPILER
int main ()
{
	InitCompiler ();
	ParserInfo p = compile ("Pong");

	if (p.er != none)
		printf("Compilation failed\n");
	else
		printf("Compilation successful\n");

	StopCompiler ();
	return 1;
}
#endif
