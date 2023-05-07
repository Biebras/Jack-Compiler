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
#include <sys/stat.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "compiler.h"

static bool secondPass = false;

char** buildInFiles = NULL;
int buildInFileCount = 0;
char** directoryFiles = NULL;
int directoryFileCount = 0;

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
        char path[1024];
        struct stat info;
        snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);
        if (!stat(path, &info))
        {
            if (S_ISREG(info.st_mode))
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
    }

    closedir(dir);
    *numFiles = count;
    return files;
}

int InitCompiler ()
{
	InitSymbolTable();

	return 1;
}

ParserInfo ParseBuildInFiles()
{
	ParserInfo p;
	p.er = none;

	for (int i = 0; i < buildInFileCount; i++)
	{
		InitParser(buildInFiles[i]);
		p = Parse();
		StopParser();

		if (p.er != none)
		{
			return p;
		}
	}

	return p;
}

ParserInfo ParseDirectoryFiles(char* dir_name)
{
	ParserInfo p;
	p.er = none;

	for (int i = 0; i < directoryFileCount; i++)
	{
		char filePath[1024];
		snprintf(filePath, sizeof(filePath), "%s/%s", dir_name, directoryFiles[i]);

		InitParser(filePath);
		p = Parse();
		StopParser();

		if (p.er != none)
		{
			return p;
		}
	}

	return p;
}

ParserInfo ParseFiles(char* dir_name)
{
	ParserInfo p;
	p.er = none;

	p = ParseBuildInFiles();

	if (p.er != none)
		return p;

	p = ParseDirectoryFiles(dir_name);

	if (p.er != none)
		return p;

	// If second pass is true, then we don't need to check for undeclared symbols.
	if (secondPass == true)
		return p;

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

ParserInfo compile (char* dir_name)
{
	printf("Starting compiler...\n");

	ParserInfo p;
	p.er = none;

	buildInFiles = getJackFiles(".", &buildInFileCount);
	directoryFiles = getJackFiles(dir_name, &directoryFileCount);

	p = ParseFiles(dir_name);

	if (p.er != none)
		return p;

	return p;
}

int StopCompiler ()
{
	//PrintSymbolTable();
	printf("Compiler stopped.\n");
	FreeSymbolTable();
	return 1;
}