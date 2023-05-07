

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

#define JsonStrSize 5000
#define Presubmission 1

// remove this before releasing template
#define NumberTestFiles 10
char* JsonStr;

Token tokenArray[2024];
int numberTokens;

int LineNum;
FILE* CurVMFile;
char CurVMFileName [128];

#define NumCommands  17
char* Commands[NumCommands]= {"add" , "sub" ,"neg" ,"eq" ,"gt" ,"lt" ,"and" ,"or" ,"not" ,"push" ,"pop" ,"label" ,"goto" , "if-goto" , "function" , "call" , "return" };

char* code_files[128];  // list of code files to be checked

char* testPrograms[NumberTestFiles] = {
	"Seven",
	"Fraction",
	"HelloWorld",
	"Square",
	"Average",
	"ArrayTest",
	"MathTest",
	"List",
	"ConvertToBin",
	"Pong"
};

int InitVMLexer (char* file_name)
{
  CurVMFile = fopen (file_name, "r");
  if (CurVMFile == 0)
  {
    printf ("Unable to open code file %s\n", file_name);
    return (0);
  }
  strcpy (CurVMFileName , file_name);
  LineNum = 1;
  return 1;
}

int StopVMLexer ()
{
	fclose (CurVMFile);
	return 1;
}


int InitGraderString ()
{
	JsonStr = (char *) malloc (sizeof (char) * JsonStrSize);
	strcpy (JsonStr, "{\n");   // let's start, bismillah
	strcat (JsonStr, "\t\"output\": \"Graded by CAutoGrader\",\n");
	strcat (JsonStr, "\t\"stdout_visibility\": \"visible\",\n");
	strcat (JsonStr, "\t\"tests\":\n");  // let's start the tests
	strcat (JsonStr, "\t[\n");
	return 1;
}

int AddTestString (int score, int max_score, char* output, int is_final)
{
	char temp[320];

	strcat (JsonStr , "\t\t{\n");
	sprintf (temp, "%i", score);
	strcat (JsonStr , "\t\t\t\"score\": ");
	strcat (JsonStr , temp);
	strcat (JsonStr , ",\n");

	sprintf (temp, "%i", max_score);
	strcat (JsonStr , "\t\t\t\"max_score\": ");
	strcat (JsonStr , temp);
	strcat (JsonStr , ",\n");


	strcat (JsonStr , "\t\t\t\"output\": ");
	strcat (JsonStr , "\"");
	strcat (JsonStr , output);
	strcat (JsonStr , "\"");
	strcat (JsonStr , "\n");

	strcat (JsonStr , "\t\t}");
	if (!is_final)
		strcat (JsonStr, ",");
	strcat (JsonStr, "\n");

	return 1;
}


// get a list of code (vm) files in a compiled jack program given by its directory name (dir_name)
int GetCodeFiles (char* dir_name)
{
	char tmp[256];
	sprintf (tmp, "ls %s > SourceFiles.txt", dir_name);
	system (tmp);
	// use my lexer to get the file names
	InitLexer ("SourceFiles.txt");
	Token t =  PeekNextToken ();
	int i = 0;
	while (t.tp != EOFile)
	{
		t = GetNextToken(); // get the file name without extension
		strcpy (tmp , t.lx);
		t = GetNextToken() ; // get the .
		strcat (tmp, t.lx);
		t = GetNextToken();  // get the extension
		strcat (tmp, t.lx);
		// if the extension is vm then add this file to the list
		if (!strcmp(t.lx, "vm"))
		{
			code_files[i]= (char*) malloc (sizeof (char) * (strlen (tmp)  ));
			strcpy (code_files[i++], tmp);
		}
		t = PeekNextToken();
	}
	StopLexer();
	return i;
}



int IsVMCommand (char* word)
{
  for (int i = 0; i < NumCommands ; i++)
    if (strcmp (Commands[i],word) == 0)
		return 1;
  return 0;
}

// eat white space
int EatSpace ()
{
  int c;
  c = getc (CurVMFile);  // get the next char from the input file
  while (c != EOF && isspace (c) ) // keep eating white space
  {
	if (c == '\n')
		LineNum++;
    c = getc (CurVMFile); // move ahead and get next char
  }
  // when main loop  is broken then c contains next non-space char
  return c;
}


Token GetVMToken ()
{
  int c;
  Token t;
  char s[256];
  int i;

  strcpy (t.fl , CurVMFileName);
  // consume leading white-space
  c =  EatSpace();

  if (c == EOF)
  {
    t.tp = EOFile;
	strcpy (t.lx , "End of File");
	t.ln = LineNum;
    return t;
  }
  else if (isdigit (c)) // a number
  {
    i=0;
    while (c != EOF && isdigit (c))
    {
      s[i++] = c;
      c = getc (CurVMFile);
    }
    ungetc(c, CurVMFile);
    s[i] = '\0';
    t.tp = INT;
    strcpy (t.lx , s);
	t.ln = LineNum;
    return t;
  }
  else if (isalpha (c)) //|| c == '-' || c == '_'
  {
    i=0;
    while (c != EOF && (isalpha (c) || isdigit (c) || c == '-' || c == '_'))
    {
      s[i++] = c;
      c = getc (CurVMFile);
    }
    ungetc(c, CurVMFile);
    s[i] = '\0';
	if (IsVMCommand(s))
      t.tp = RESWORD;
    else
      t.tp = ID;
    strcpy (t.lx , s);
	t.ln = LineNum;
    return t;
  }
  else
  {
	if (c != '.' )
	{
      t.tp = ERR;
	  strcpy (t.lx , "Error: illegal symbol in source file");
	  t.ec = IllSym;
	  t.ln = LineNum;
      return t;
	}
    t.tp = SYMBOL;
    s[0] = c;
    s[1] = '\0';
    strcpy (t.lx , s);
	t.ln = LineNum;
    return t;
  }
}

int CloseGraderString ()
{
	strcat (JsonStr, "	]\n"); // end of tests
	strcat (JsonStr, "}\n");
	return 1;
}

char* TokenTypeString (TokenType t)
{
	switch (t)
	{
		case RESWORD: return "RESWORD";
		case ID: return "ID";
		case INT: return "INT";
		case SYMBOL: return "SYMBOL";
		case STRING: return "STRING";
		case EOFile: return "EOFile";
		case ERR: return "ERR";
		default: return "Not a recognised token type";
	}

}

char* ErrorString (SyntaxErrors e)
{

	switch (e)
	{
		case none: return "no errors";
		case lexerErr: return "lexer error";
		case classExpected: return "keyword class expected";
		case idExpected: return "identifier expected";
		case openBraceExpected:	return "{ expected";
		case closeBraceExpected: return "} expected";
		case memberDeclarErr: return "class member declaration must begin with static, field, constructor , function , or method";
		case classVarErr: return "class variables must begin with field or static";
		case illegalType: return "a type must be int, char, boolean, or identifier";
		case semicolonExpected: return "; expected";
		case subroutineDeclarErr: return "subrouting declaration must begin with constructor, function, or method";
		case openParenExpected: return "( expected";
		case closeParenExpected: return ") expected";
		case closeBracketExpected: return "] expected";
		case equalExpected: return "= expected";
		case syntaxError: return "syntax error";
		// semantic errors
		case undecIdentifier: return "undeclared identifier";
		case redecIdentifier: return "redeclaration of identifier";
		default: return "not a valid error code";
	}
}


void PrintError (ParserInfo pn)
{
	if (pn.er == none)
		printf ("No errors\n");
	else
		printf ("Error in file %s line %i at or near %s: %s\n" , pn.tk.fl , pn.tk.ln , pn.tk.lx , ErrorString(pn.er));
}


void ShowInfo (ParserInfo pn)
{
	if (pn.er == none)
		printf ("none\n");
	else
		printf ("error type: %s, line: %i,token: %s, \n" , ErrorString(pn.er),  pn.tk.ln , pn.tk.lx  );
}


void PrintToken (Token t)
{
	printf ("<%s, %i, %s, %s>\n", t.fl, t.ln , t.lx, TokenTypeString (t.tp));
}


// test the parser
int t_compiler ()
{
	int m=20;
	char s[100];

	printf ("\nTesting your compiler on various JACK programs (2 marks each)\n");
	for (int j = 0 ; j < NumberTestFiles ; j++) // for each test file
	{
		printf ("Running your compiler on program %s \n", testPrograms[j]);
		InitCompiler ();
		ParserInfo p = compile (testPrograms[j]);
		StopCompiler ();
		if (p.er != none)
		{
			printf ("** Oops: your compiler failed to compile the program and returned the following error:\n");
			PrintError (p);
			printf ("Sorry, -2 mark\n");
			m=m-2;
		}
		else
		{
			// get the files of the benchmark compilation
			char full_dir_name[128];
			strcpy (full_dir_name , testPrograms[j]);
			strcat (full_dir_name , "_compiled");
			int num_vm_files = GetCodeFiles (full_dir_name);
			for (int k = 0; k < num_vm_files ; k++)  // compare each vm file to the one generated by the compiler
			{
				printf ("Checking file %s\n", code_files[k]);
				// get the tokens of the benchmark file
				char vm_file_name[128];
				strcpy (vm_file_name , testPrograms[j]);
				strcat (vm_file_name , "_compiled/");
				strcat (vm_file_name , code_files[k]);
				int r  = InitVMLexer (vm_file_name);
				if (r == 0)
				{
					printf ("Fatal autograder error, unable to open a vm file\n");
					exit (0);
				}
				// now extract the tokens of the standard vm file an store in an array
				numberTokens = 0;
				Token t = GetVMToken ();
				while (t.tp != EOFile)
				{
					tokenArray[numberTokens++] = t;
					t = GetVMToken ();
				}
				StopVMLexer();
				//  now compare the tokens with those of the file created by the compiler
				strcpy (vm_file_name , testPrograms[j]);
				strcat (vm_file_name , "/");
				strcat (vm_file_name, code_files[k]);
				r = InitVMLexer (vm_file_name);
				if (r == 0)
				{
					printf ("Unable to open one of your vm files. Did your compiler create all code files?\n");
					m-=2;
					StopVMLexer();
					break;
				}
				int i = 0;
				t = GetVMToken ();
				Token prev_token;
				strcpy (prev_token.lx , "nothing");
				while (t.tp != EOFile)
				{
					//printf ("%s =? %s\n", t.lx, tokenArray[i].lx);
					if (!strcmp (prev_token.lx,"label") || !strcmp (prev_token.lx,"goto") || !strcmp (prev_token.lx,"if-goto") )
					{
						;  // ignore labels
					}
					else if ( strcmp(t.lx , tokenArray[i].lx))
					{
						printf ("Error in code file\n");
						printf ("Expecting %s in line %i\n" , tokenArray[i].lx, tokenArray[i].ln);
						printf ("Found %s in line %i instead\n", t.lx, t.ln);
						printf ("Sorry, -2 marks\n");
						m=m-2;
						break;
					}
					i++;
					prev_token = t;
					t = GetVMToken ();
				}
				StopVMLexer ();
				printf ("*** PASSED *** \n");
			}
		}
	}
	sprintf (s,"%i/20 for code generation", m);
	if (!Presubmission)
		AddTestString (m, 20, s, 1);
	return m;
}


#ifdef TEST_COMPILER
int main (int argc, char* argv[])
{
	FILE* jsonFile;
	int tot = 0;


	if (!Presubmission)
		InitGraderString ();

	printf ("\t$$$ Checking your compiler, behold $$$\n");
	printf ("\t=========================================\n");
	printf ("Started ...\n");

	tot += t_compiler ();

	if (Presubmission)
	{
		printf ("\n---------------------------------------------------\n");
		printf ("\t\tTotal mark = %i/20\n", tot);
		printf ("---------------------------------------------------\n\n");
	}
	printf ("Finished\n");


	if (!Presubmission)  // create the results.json file
	{
		CloseGraderString ();
		//printf ("%s",JsonStr);
		//jsonFile = fopen ("/autograder/results/results.json", "w");
		//fprintf (jsonFile, "%s", JsonStr);
		//fclose (jsonFile);
	}


	return 0;
}
#endif
