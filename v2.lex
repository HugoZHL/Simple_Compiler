/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/17
*/

%{
	#include "tigger2riscv.h"
	#include "y.tab.h"

%}

integer [1-9][0-9]*|"0"
letter [a-zA-Z]
iden (_|{letter})(_|[0-9]|{letter})*
space [ \t]+
enter [\n]
comment "//"[^\n]*
function f_{iden}
register (v|s|t|a){integer}
label l{integer}


%%
{comment} 	{}
{space} 	{}
{enter} 	{}
store		{return STORE;}
call		{return CALL;}
malloc		{return MALLOC;}
end			{return END;}
if			{return IF;}
goto		{return GOTO;}
loadaddr	{return LOADADDR;}
load		{return LOAD;}
return		{return RETURN;}
{register}	{yylval = strdup(yytext); return REGNAME;}
{function} 	{yylval = strdup(yytext+2); return FUNCNAME;}
{label}		{yylval = strdup(yytext+1); return LABNAME;}
{integer} 	{yylval = strdup(yytext); return IMMNAME;}
"[" 		{return LMB;}
"]"			{return RMB;}
"!"			{return NOT;}
"-"			{return MIN;}
"="			{return ASS;}
"!="		{yylval = strdup(yytext); return OP2;}
"=="		{yylval = strdup(yytext); return OP2;}
">"			{yylval = strdup(yytext); return OP2;}
"<"			{yylval = strdup(yytext); return OP2;}
"&&"		{yylval = strdup(yytext); return OP2;}
"||"		{yylval = strdup(yytext); return OP2;}
"+"			{yylval = strdup(yytext); return OP2;}
"*"			{yylval = strdup(yytext); return OP2;}
"/"			{yylval = strdup(yytext); return OP2;}
"%"			{yylval = strdup(yytext); return OP2;}
":"			{return COL;}
%%



int yywrap()
{
	return 1;
}
