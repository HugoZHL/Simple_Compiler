/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/11/3
*/

%{
	#include "./eeyore_file/parse.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	static Node * root;
	int yylex();
	int yyerror(char*);
%}
%token TYPE IF ELSE WHILE RETURN MAIN IMM IDEN ADD SUB MUL DIV MOD
%token ASG NOT LTH GTH AND OR EQU NEQ LBS RBS LBM RBM LBL RBL COM SEM


%%
Root:
	  Goal 	{$$=$1; root=$1;}
Goal:
	  VarDefn Goal 		{$$=$1; $$->sib=$2;}
	| FuncDefn Goal 	{$$=$1; $$->sib=$2;}
	| FuncDecl Goal 	{$$=$1; $$->sib=$2;}
	| MainFunc			{$$=$1;}
	;
VarDefn:
	  TYPE IDEN SEM 			{$$=makeNode("int;", $2, _vdef);}
	| TYPE IDEN LBM IMM RBM SEM {$$=makeNode("int[];", $2, _vdef); $2->sib=$4;}
	;
VarDecl:
	  TYPE IDEN 			{$$=makeNode("int", $2, _vdcl);}
	| TYPE IDEN LBM RBM 	{$$=makeNode("int[]", $2, _vdcl);}
	| TYPE IDEN LBM IMM RBM {$$=makeNode("int[]", $2, _vdcl); $2->sib=$4;}
	;
ParamList:
	  VarDecl					{$$=$1;}
	| VarDecl COM ParamList 	{$$=$1; $$->sib=$3;}
	| 							{$$=NULL;}
	;
FuncDecl:
	  TYPE IDEN LBS ParamList RBS SEM 	{$$=makeNode("func;", $2, _fdcl); $2->sib=$4;}
	;
FuncStat:
	  FuncDecl FuncStat 	{$$=$1; $$->sib=$2;}
	| Statement FuncStat 	{$$=$1; $$->sib=$2;}
	| 						{$$=NULL;}
	;
FuncDefn:
	  TYPE IDEN LBS ParamList RBS LBL FuncStat RBL 
	  	{
	  		$$=makeNode("func", $2, _fdef);
	  		$2->sib=$4;
	  		rightMost($2)->sib=$7;
	  	}
	;
MainFunc:
	  TYPE MAIN LBS RBS LBL FuncStat RBL 	{$$=makeNode("main", $6, _main);}
	;
IdenList:
	  Expression 				{$$=$1;}
	| Expression COM IdenList 	{$$=$1; $$->sib=$3;}
	| 							{$$=NULL;}
	;
Statement:
	  IF LBS Expression RBS Statement
		{
			$$=makeNode("if", $3, _stat);
			$3->sib=$5;
		}
	| IF LBS Expression RBS WithoutIF ELSE Statement
		{
			$$=makeNode("ifel", $3, _stat);
			$3->sib=$5;
			$5->sib=$7;
		}
	| WithoutIF 	{$$=$1;}
Statements:
	  Statement Statements 	{$$=$1; $$->sib=$2;}
	| 						{$$=NULL;}
	;
WithoutIF:
	  LBL Statements RBL 		{$$=makeNode("{}", $2, _stat);}
	| WHILE LBS Expression RBS Statement
		{
			$$=makeNode("while", $3, _stat);
			$3->sib=$5;
		}
	| IDEN ASG Expression SEM 	{$$=makeNode("int=", $1, _stat); $1->sib=$3;}
	| IDEN LBM Expression RBM ASG Expression SEM
		{
			$$=makeNode("int[]=", $1, _stat);
			$1->sib=$3;
			$3->sib=$6;
		}
	| VarDefn 					{$$=$1;}
	| IDEN LBS IdenList RBS SEM {$$=makeNode("call", $1, _stat); $1->sib=$3;}
	| RETURN Expression SEM 	{$$=makeNode("return", $2, _stat);}
	;
Expression:
	  Expression OR Expr1	{$$=makeNode("||", $1, _expr); $1->sib=$3;}
	| Expr1 				{$$=$1;}
	;
Expr1:
	  Expr1 AND Expr2 	{$$=makeNode("&&", $1, _expr); $1->sib=$3;}
	| Expr2				{$$=$1;}
	;
Expr2:
	  Expr2 EQU Expr3 	{$$=makeNode("==", $1, _expr); $1->sib=$3;}
	| Expr2 NEQ Expr3 	{$$=makeNode("!=", $1, _expr); $1->sib=$3;}
	| Expr3 			{$$=$1;}
	;
Expr3:
	  Expr3 LTH Expr4 	{$$=makeNode("<", $1, _expr); $1->sib=$3;}
	| Expr3 GTH Expr4 	{$$=makeNode(">", $1, _expr); $1->sib=$3;}
	| Expr4 			{$$=$1;}
	;
Expr4:
	  Expr4 ADD Expr5 	{$$=makeNode("+", $1, _expr); $1->sib=$3;}
	| Expr4 SUB Expr5 	{$$=makeNode("-", $1, _expr); $1->sib=$3;}
	| Expr5 			{$$=$1;}
	;
Expr5:
	  Expr5 MUL Expr6 	{$$=makeNode("*", $1, _expr); $1->sib=$3;}
	| Expr5 DIV Expr6 	{$$=makeNode("/", $1, _expr); $1->sib=$3;}
	| Expr5 MOD Expr6	{$$=makeNode("%", $1, _expr); $1->sib=$3;}
	| Expr6 			{$$=$1;}
	;
Expr6:
	  NOT Expr7 	{$$=makeNode("not", $2, _expr);}
	| SUB Expr7 	{$$=makeNode("neg", $2, _expr);}
	| Expr7 		{$$=$1;}
	;
Expr7:
	  IDEN LBM Expression RBM 	{$$=makeNode("[]", $1, _expr); $1->sib=$3;}
	| LBS Expression RBS 		{$$=$2;}
	| IDEN LBS IdenList RBS 	{$$=makeNode("call", $1, _expr); $1->sib=$3;}
	| IMM 						{$$=$1;}
	| IDEN 						{$$=$1;}
	;
%%


int main(int argc, char* argv[])
{
	char buf[1024];
	char ch;
	while(~scanf("%[^\n]", buf))
	{
		fprintf(stderr, "%s\n", buf);
		scanf("%c", &ch);
	}
	rewind(stdin);
	_line_num=0;
	yyparse();
	scanTree(root);
	return 0;
}

int yyerror(char* error)
{
	fprintf(stderr, "Line %d: syntax error: %s\n", _line_num, error);
	exit(1);
}
