/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/11/3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum _node_type
{
	_resv = 0,	//reserved node
	_vdcl = 1,	//variable declare
	_fdcl = 2,	//function declare
	_vdef = 3,	//variable define
	_fdef = 4,	//function define
	_main = 5,	//main funciton
	_stat = 6,	//statement
	_expr = 7,	//expression
	_iden = 8,	//identifier
	_imme = 9,	//immediate number
};

enum _var_type
{
	_vari = 0,	//variable
	_para = 1,	//parameter
	_temp = 2,	//temporary use
	_inst = 3, 	//immediate number
};

struct NODES;
typedef struct NODES
{
	enum _node_type type;
	struct NODES* son;	//left son
	struct NODES* sib;	//right sibling
	const char* attr;	//attribute
	int linenum;		//line number
}Node;

typedef struct
{
	enum _var_type type;
	char* oriName;	//original variable name
	char* yoreName;	//transferred variable name
	int isarray;	//check if array
	int valid; 		//check if valid in this environment
	int linenum;	//line number
	int arraysize;	//size of array
}Variable;

struct PARAM;
typedef struct PARAM
{
	int isarray;			//check if the parameter is array
	int arraysize;			//size of array
	struct PARAM* next;		//next parameter
}parameter;

typedef struct 
{
	char* funcName;		//function name
	int linenum;		//line number
	int declared; 		//check if declared
	parameter* first;	//the first parameter
}Func;


Node* makeNode(const char*, Node*, enum _node_type);
Node* rightMost(Node*);
void scanTree(Node* root);
void printTree(Node*);

static int _line_num;

#define YYSTYPE Node*
