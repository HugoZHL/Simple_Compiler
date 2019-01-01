/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/1
*/

%{
	#include "parse2tigger.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	int global;
	int yylex();
	int yyerror(char*);
%}
%token VAR CAL PAR RET END IFS GOT LMB RMB NOT MIN ASS OP2 COL
%token VARNAME FUNCNAME LABNAME IMMNAME

%%
Goal: 
	  Statement Goal 	{}
	| Statement 		{}
	;
Statement:
	  FUNCNAME LMB IMMNAME RMB 	
	  	{
	  		makeEXPR(_func, $1, $3, NULL, NULL);
	  		global = false;
	  	}
	| VAR VARNAME	
		{
			if(global == true)
			{
				addVar($2, true, false, NULL);
				makeEXPR(_decl, $2, "", NULL, NULL);
			}
			else
			{
				addVar($2, false, false, NULL);
				makeEXPR(_decl, $2, NULL, NULL, NULL);
			}
		}
	| VAR IMMNAME VARNAME
		{
			if(global == true)
			{
				addVar($3, true, true, $2);
				makeEXPR(_decl, $3, "", "", $2);
			}
			else
			{
				addVar($3, false, true, $2);
				makeEXPR(_decl, $3, NULL, "", $2);
			}
		}
	| VARNAME ASS RightValue OP2 RightValue 	{makeEXPR(_ope2, $4, $3, $5, $1);}
	| VARNAME ASS RightValue MIN RightValue 	{makeEXPR(_ope2, "-", $3, $5, $1);}
	| VARNAME ASS MIN RightValue 				{makeEXPR(_ope1, "-", $4, NULL, $1);}
	| VARNAME ASS NOT RightValue				{makeEXPR(_ope1, "!", $4, NULL, $1);}
	| VARNAME ASS RightValue					{makeEXPR(_assi, $3, $1, "0", NULL);}
	| VARNAME LMB RightValue RMB ASS RightValue {makeEXPR(_assi, $6, $1, "1", $3);}
	| VARNAME ASS VARNAME LMB RightValue RMB	{makeEXPR(_assi, $3, $1, "2", $5);}
	| IFS RightValue OP2 RightValue GOT LABNAME	{makeEXPR(_ifbr, $3, $2, $4, $6);}
	| GOT LABNAME								{makeEXPR(_goto, $2, NULL, NULL, NULL);}
	| LABNAME COL								{makeEXPR(_plab, $1, NULL, NULL, NULL);}
	| PAR RightValue							{makeEXPR(_para, $2, NULL, NULL, NULL);}
	| CAL FUNCNAME 								{makeEXPR(_call, NULL, $2, NULL, NULL);}
	| VARNAME ASS CAL FUNCNAME					{makeEXPR(_call, $1, $4, NULL, NULL);}
	| RET RightValue							{makeEXPR(_retu, $2, NULL, NULL, NULL);}
	| END FUNCNAME								{makeEXPR(_ends, $2, NULL, NULL, NULL); global = true;}
	;
RightValue:
	  VARNAME	{$$=$1;}
	| IMMNAME	{$$=$1;}
	;
%%


int main(int argc, char* argv[])
{
	global = true;
#ifndef testing
	char buf[1024];
	char ch;
	while(~scanf("%[^\n]", buf))
	{
		fprintf(stderr, "%s\n", buf);
		scanf("%c", &ch);
	}
	rewind(stdin);
#endif

#ifdef testing
	freopen("test.txt", "r", stdin);
	freopen("out.txt", "w", stdout);
	//freopen("error.txt", "w", stderr);
#endif

	exprIndex = 0;
	varIndex = 0;
	resultIndex = 0;
	globalIndex = 0;
	yyparse();


#ifdef testing
	//testExprList();
#endif

	updateTable();

#ifdef testing
	//fprintf(stdout, "update already\n");
	//testActtab();
#endif

	printResult();

#ifdef testing
	//fprintf(stdout, "print result already\n");
	fclose(stdin);
	fclose(stdout);
#endif

	return 0;
}

int yyerror(char* error)
{
	fprintf(stderr, "Syntax error: %s\n", error);
	exit(1);
}

