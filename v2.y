/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/17
*/

%{
	#include "tigger2riscv.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	int global;
	int yylex();
	int yyerror(char*);
%}
%token MALLOC END IF GOTO CALL STORE LOAD LOADADDR RETURN
%token REGNAME FUNCNAME LABNAME IMMNAME 
%token LMB RMB NOT MIN ASS OP2 COL

%%
Goal: 
	  Statement Goal 	{}
	| Statement 		{}
	;
Statement:
	  FUNCNAME LMB IMMNAME RMB LMB IMMNAME RMB	{makeEXPR(_func, $1, $3, $6, NULL);}
	| REGNAME ASS MALLOC IMMNAME				{makeEXPR(_decl, $1, $4, NULL, NULL);}
	| REGNAME ASS REGNAME OP2 REGNAME			{makeEXPR(_ope2, $4, $3, $5, $1);}
	| REGNAME ASS REGNAME MIN REGNAME			{makeEXPR(_ope2, "-", $3, $5, $1);}
	| REGNAME ASS MIN REGNAME 					{makeEXPR(_ope1, "-", $4, NULL, $1);}
	| REGNAME ASS NOT REGNAME					{makeEXPR(_ope1, "!", $4, NULL, $1);}
	| REGNAME ASS REGNAME 						{makeEXPR(_assi, $1, $3, NULL, NULL);}
	| REGNAME ASS IMMNAME 						{makeEXPR(_assi, $1, $3, NULL, NULL);}
	| REGNAME LMB IMMNAME RMB ASS REGNAME 		{makeEXPR(_assi, $1, $6, $3, NULL);}
	| REGNAME ASS REGNAME LMB IMMNAME RMB  		{makeEXPR(_assi, $1, $3, NULL, $5);}
	| IF REGNAME OP2 REGNAME GOTO LABNAME 		{makeEXPR(_ifbr, $3, $2, $4, $6);}
	| GOTO LABNAME 								{makeEXPR(_goto, $2, NULL, NULL, NULL);}
	| LABNAME COL 								{makeEXPR(_plab, $1, NULL, NULL, NULL);}
	| CALL FUNCNAME 							{makeEXPR(_call, $2, NULL, NULL, NULL);}
	| STORE REGNAME IMMNAME 					{makeEXPR(_stor, $2, $3, NULL, NULL);}
	| LOAD IMMNAME REGNAME 						{makeEXPR(_load, $2, $3, NULL, NULL);}
	| LOAD REGNAME REGNAME 						{makeEXPR(_load, $2, $3, NULL, NULL);}
	| LOADADDR IMMNAME REGNAME 					{makeEXPR(_ldad, $2, $3, NULL, NULL);}
	| LOADADDR REGNAME REGNAME 					{makeEXPR(_ldad, $2, $3, NULL, NULL);}
	| RETURN 									{makeEXPR(_retu, NULL, NULL, NULL, NULL);}
	| END FUNCNAME 								{makeEXPR(_endf, $2, NULL, NULL, NULL);}
	;
%%


int main(int argc, char* argv[])
{

#ifdef testing
	freopen("test.txt", "r", stdin);
	//freopen("out.txt", "w", stdout);
	//freopen("error.txt", "w", stderr);
#endif
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
	
	exprIndex = 0;
	yyparse();

	translate();

	return 0;
}

int yyerror(char* error)
{
	fprintf(stderr, "Syntax error: %s\n", error);
	exit(1);
}

