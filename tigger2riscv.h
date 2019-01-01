/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#define true 1
#define false 0
//#define testing

enum _expr_type
{
	_func = 0,
	_decl = 1,
	_ope2 = 2,
	_ope1 = 3,
	_assi = 4,
	_ifbr = 5,
	_goto = 6,
	_plab = 7,
	_call = 8,
	_stor = 9,
	_load = 10,
	_ldad = 11,
	_retu = 12,
	_endf = 13,
};	//to show what type the expression is

typedef struct EXPRS
{
	int index;
	enum _expr_type type;
	const char* op;
	const char* src1;
	const char* src2;
	const char* dst;
}Expr;	//the structure of expression

static int exprIndex;

void makeEXPR(enum _expr_type, const char*, const char*, const char*, const char*);
void translate();

#define YYSTYPE const char*