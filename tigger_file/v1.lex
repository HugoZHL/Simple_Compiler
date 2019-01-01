/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/1
*/

%{
	#include "parse2tigger.h"
	#include "y.tab.h"

%}

integer [1-9][0-9]*|"0"
letter [a-zA-Z]
iden (_|{letter})(_|[0-9]|{letter})*
space [ \t]+
enter [\n]
comment "//"[^\n]*
function f_{iden}
variable (T|t|p){integer}
label l{integer}


%%
{comment} 	{}
{space} 	{}
{enter} 	{}
var			{return VAR;}
call		{return CAL;}
param		{return PAR;}
return		{return RET;}
end			{return END;}
if 			{return IFS;}
goto		{return GOT;}
{variable}	{yylval = strdup(yytext); return VARNAME;}
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
