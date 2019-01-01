/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/11/3
*/

%{
	#include "./eeyore_file/parse.h"
	#include "y.tab.h"

%}

integer [1-9][0-9]*|"0"
letter [a-zA-Z]
iden (_|{letter})(_|[0-9]|{letter})*
space [ \t]+
enter [\n]
comment "//"[^\n]*


%%
int 		{return TYPE;}
{comment} 	{}
{space} 	{}
{enter} 	{_line_num++;}
if 			{return IF;}
else 		{return ELSE;}
while 		{return WHILE;}
return 		{return RETURN;}
main 		{return MAIN;}
{integer} 	{yylval = makeNode(strdup(yytext), NULL, _imme); return IMM;}
{iden} 		{yylval = makeNode(strdup(yytext), NULL, _iden); return IDEN;}
"+"			{return ADD;}
"-"			{return SUB;}
"*"			{return MUL;}
"/"			{return DIV;}
"%"			{return MOD;}
"="			{return ASG;}
"!"			{return NOT;}
"<"			{return LTH;}
">"			{return GTH;}
"&&"		{return AND;}
"||"		{return OR;}
"=="		{return EQU;}
"!="		{return NEQ;}
"("			{return LBS;}
")"			{return RBS;}
"["			{return LBM;}
"]"			{return RBM;}
"{"			{return LBL;}
"}"			{return RBL;}
","			{return COM;}
";"			{return SEM;}

%%



int yywrap()
{
	return 1;
}
