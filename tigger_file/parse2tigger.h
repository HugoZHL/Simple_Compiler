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
	_para = 8,
	_call = 9,
	_retu = 10,
	_ends = 11,
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

typedef struct VARS
{
	int index;
	const char* name;
	short global;
	short array;
	int size;
	const char* globalname;
}Var; 	//the structure of variable

typedef enum _result_type
{
	_rfunc = 0,
	_rdecl = 1,
	_rope2 = 2,
	_rope1 = 3,
	_rassi = 4,
	_rifbr = 5,
	_rgoto = 6,
	_rplab = 7,
	_rcall = 8,
	_rstor = 9,
	_rload = 10,
	_rldad = 11,
	_rretu = 12,
	_rends = 13,
}RTYPE; 

typedef struct RESULT
{
	int index;
	RTYPE type;
	const char* op;
	const char* src1;
	const char* src2;
	const char* dst;
}ResEntry;	//the structure of result expression

static int exprIndex;
static int varIndex;
static int resultIndex;
static int globalIndex;

void addVar(const char*, short, short, const char*);
void makeEXPR(enum _expr_type, const char*, const char*, const char*, const char*);
void updateTable();
void printResult();
void testExprList();
void testActtab();

#define YYSTYPE const char*