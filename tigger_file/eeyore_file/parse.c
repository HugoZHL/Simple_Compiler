/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/11/3
*/

#include "parse.h"
#define false 0
#define true 1
#define unknown 2
#define EQ(a,b) (strcmp(a,b)==0)

int VarCnt, TempCnt, ParamCnt;
int LabelCnt, FuncCnt;
Variable* VarList[2000];
Variable* ParamList[20];
Func* FuncList[1000];
int LayerMark[100];
int currLayer;
int indent;

void scanNode(Node* node);
Variable* scanEXPR(Node* node);

void initEnv()
{
	VarCnt = TempCnt = ParamCnt = 0;
	LabelCnt = FuncCnt = 0;
	currLayer = indent = 0;
	LayerMark[0] = 0;
}

Node* makeNode(const char* attr, Node* son, enum _node_type type)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->type = type;
	newNode->son = son;
	newNode->sib = NULL;
	newNode->attr = attr;
	newNode->linenum = _line_num;
	return newNode;
}

Variable* makeVar(const char* oriName, int isarray, enum _var_type type, int linenum, int arraysize)
{
	Variable* newVar = (Variable*)malloc(sizeof(Variable));
	newVar->type = type;
	newVar->isarray = isarray;
	newVar->linenum = linenum;
	newVar->valid = true;
	newVar->oriName = (oriName==NULL?NULL:strdup(oriName));
	newVar->arraysize = arraysize;
	if(type == _inst)
	{
		if(isarray || oriName==NULL)
		{
			fprintf(stderr, "Line %d: type error: %s\n", linenum, oriName);
			exit(1);
		}
		newVar->yoreName = strdup(oriName);
		return newVar;
	}
	else
	{
		char buffer[10] = {'\0'};
		if(type == _vari)
		{
			sprintf(buffer, "T%d", VarCnt);
			VarList[VarCnt++] = newVar;
		}
		else if(type == _temp) sprintf(buffer, "t%d", TempCnt++);
		else
		{
			sprintf(buffer, "p%d", ParamCnt);
			ParamList[ParamCnt++] = newVar;
		}
		newVar->yoreName = strdup(buffer);
		if(type != _para)
		{
			if(indent) fprintf(stdout, "	");
			if(!isarray) fprintf(stdout, "var %s\n", newVar->yoreName);
			else fprintf(stdout, "var %d %s\n", 4*arraysize, newVar->yoreName);
		}	
		return newVar;
	}
}

Variable* matchVar(const char* oriName, int isarray, enum _var_type type, int linenum)
{
	if(type == _vari)
	{
		int i;
		for(i = 0; i < ParamCnt; ++i)
			if(EQ(ParamList[i]->oriName,oriName))
			{
				if(isarray != unknown && isarray != ParamList[i]->isarray)
				{
					fprintf(stderr, "Line %d: type error: %s\n", linenum, oriName);
					exit(1);
				}
				return ParamList[i];
			}
		for(i = VarCnt-1; i >= 0; --i)
		{
			if(EQ(VarList[i]->oriName,oriName) && VarList[i]->valid)
			{
				if(isarray != unknown && isarray != VarList[i]->isarray)
				{
					fprintf(stderr, "Line %d: type error: %s\n", linenum, oriName);
					exit(1);
				}
				return VarList[i];
			}
		}
		fprintf(stderr, "Line %d: variable not defined: %s\n", linenum, oriName);
		exit(1);
	}
	else
	{
		fprintf(stderr, "Line %d: type error: %s\n", linenum, oriName);
		exit(1);
	}
}


void checkParameter(Node* node, parameter* param, const char* funcName, int undefined)
{
	while(node && (undefined || param))
	{
		Variable* temp = scanEXPR(node);
		if((!undefined) && temp->isarray != param->isarray)
		{
			fprintf(stderr, "Line %d: function parameter type error: %s(...)\n", 
					node->linenum, funcName);
			exit(1);
		}
		if((!undefined) && (temp->isarray && param->arraysize && param->arraysize != temp->arraysize))
		{
			fprintf(stderr, "Line %d: function parameter type error: %s(...)\n",
					node->linenum, funcName);
			exit(1);
		}
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "param %s\n", temp->yoreName);
		node = node->sib;
		if(!undefined) param = param->next;
	}
	if((!undefined) && (node && node->type == _vdcl) || param)
	{
		fprintf(stderr, "Line %d: function parameter more or less: %s\n", node->linenum, funcName);
		exit(1);
	}
}

Node* rightMost(Node* node)
{
	while(node->sib) node = node->sib;
	return node;
}

Variable* scanEXPR(Node* node)
{

	if(node->type == _imme)	return makeVar(node->attr, false, _inst, node->linenum, 0);
	else if(node->type == _iden) return matchVar(node->attr, false, _vari, node->linenum);
	else
	{
		if(EQ(node->attr,"[]"))
		{
			Variable* id = matchVar(node->son->attr, true, _vari, node->linenum);
			Variable* expr = scanEXPR(node->son->sib);
			Variable* index = makeVar(NULL, false, _temp, node->linenum, 0);
			if(indent) fprintf(stdout, "	");
			fprintf(stdout, "%s = 4 * %s\n", index->yoreName, expr->yoreName);
			Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
			if(indent) fprintf(stdout, "	");
			fprintf(stdout, "%s = %s [%s]\n", temp->yoreName, id->yoreName, index->yoreName);
			return temp;
		}
		else if(EQ(node->attr,"call"))
		{
			const char* funcName = node->son->attr;
			int i;
			for(i = 0; i < FuncCnt; ++i)
				if(EQ(funcName, FuncList[i]->funcName))
				{
					checkParameter(node->son->sib, FuncList[i]->first, funcName, 0);
					Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
					if(indent) fprintf(stdout, "	");
					fprintf(stdout, "%s = call f_%s\n", temp->yoreName, funcName);
					return temp;
				}
			fprintf(stderr, "Warning: Line %d: function not defined: %s\n", node->linenum, funcName);
			checkParameter(node->son->sib, NULL, funcName, 1);
			Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
			if(indent) fprintf(stdout, "	");
			fprintf(stdout, "%s = call f_%s\n", temp->yoreName, funcName);
			return temp;
		}
		else
		{
			const char* op = node->attr;
			if(EQ(op, "neg") || EQ(op, "not"))
			{
				Variable* operand = scanEXPR(node->son);
				Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
				if(indent) fprintf(stdout, "	");
				if(EQ(op, "neg")) fprintf(stdout, "%s = -%s\n", temp->yoreName, operand->yoreName);
				else fprintf(stdout, "%s = !%s\n", temp->yoreName, operand->yoreName);
				return temp;
			}
			else
			{
				Variable* operand1 = scanEXPR(node->son);
				Variable* operand2 = scanEXPR(node->son->sib);
				Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);

				char buffer[20] = {'\0'};
				sprintf(buffer, "%s = %s %s %s", temp->yoreName, operand1->yoreName, op, operand2->yoreName);
				if(indent) fprintf(stdout, "	");
				fprintf(stdout, "%s\n", buffer);
				return temp;
			}
		}
	}
}

void scanSTAT(Node* node)
{
	const char* op = node->attr;
	if(EQ(op,"if"))
	{
		int label = LabelCnt++;
		Variable* cond = scanEXPR(node->son);
		char buffer[30] = {'\0'};
		sprintf(buffer, "if %s == 0 goto l%d\n", cond->yoreName, label);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "%s\n", buffer);
		scanNode(node->son->sib);
		fprintf(stdout, "l%d:\n", label);
		return;
	}
	else if(EQ(op,"ifel"))
	{
		int label1 = LabelCnt++;
		int label2 = LabelCnt++;
		Variable* cond = scanEXPR(node->son);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "if %s == 0 goto l%d\n", cond->yoreName, label1);
		scanNode(node->son->sib);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "goto l%d\n", label2);
		fprintf(stdout, "l%d:\n", label1);
		scanNode(node->son->sib->sib);
		fprintf(stdout, "l%d:\n", label2);
		return;
	}
	else if(EQ(op,"{}"))
	{
		LayerMark[++currLayer] = VarCnt;
		node = node->son;
		while(node)
		{
			scanNode(node);
			node = node->sib;
		}
		int i;
		for(i = LayerMark[currLayer--]; i < VarCnt; ++i)
			VarList[i]->valid = false;
		return;
	}
	else if(EQ(op,"while"))
	{
		int label1 = LabelCnt++;
		int label2 = LabelCnt++;
		fprintf(stdout, "l%d:\n", label1);
		Variable* cond = scanEXPR(node->son);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "if %s == 0 goto l%d\n", cond->yoreName, label2);
		scanNode(node->son->sib);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "goto l%d\n", label1);
		fprintf(stdout, "l%d:\n", label2);
		return;
	}
	else if(EQ(op,"int="))
	{
		Variable* operand = matchVar(node->son->attr, false, _vari, node->linenum);
		Variable* expr = scanEXPR(node->son->sib);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "%s = %s\n", operand->yoreName, expr->yoreName);
		return;
	}
	else if(EQ(op,"int[]="))
	{
		Variable* operand = matchVar(node->son->attr, true, _vari, node->son->linenum);
		Variable* index = scanEXPR(node->son->sib);
		Variable* index4 = makeVar(NULL, false, _temp, node->son->linenum, 0);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "%s = 4 * %s\n", index4->yoreName, index->yoreName);
		Variable* expr = scanEXPR(node->son->sib->sib);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "%s [%s] = %s\n", operand->yoreName, index4->yoreName, expr->yoreName);
		return;
	}
	else if(EQ(op,"call"))
	{
		const char* funcName = node->son->attr;
		int i;
		for(i = 0; i < FuncCnt; ++i)
			if(EQ(funcName, FuncList[i]->funcName))
			{
				checkParameter(node->son->sib, FuncList[i]->first, funcName, 0);
				Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
				if(indent) fprintf(stdout, "	");
				fprintf(stdout, "%s = call f_%s\n", temp->yoreName, funcName);
				return;
			}
		fprintf(stderr, "Warning: Line %d: function not defined: %s\n", node->linenum, funcName);
		checkParameter(node->son->sib, NULL, funcName, 1);
		Variable* temp = makeVar(NULL, false, _temp, node->linenum, 0);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "%s = call f_%s\n", temp->yoreName, funcName);
		return;
	}
	else if(EQ(op,"return"))
	{
		Variable* expr = scanEXPR(node->son);
		if(indent) fprintf(stdout, "	");
		fprintf(stdout, "return %s\n", expr->yoreName);
		return;
	}
}

void scanVDEF(Node* node)
{
	if(EQ(node->attr,"int;"))
		makeVar(node->son->attr, false, _vari, node->son->linenum, 0);
	else if(EQ(node->attr,"int[];"))
	{
		Variable* temp = makeVar(node->son->sib->attr, false, _inst, node->son->sib->linenum, 0);
		makeVar(node->son->attr, true, _vari, node->son->linenum, atoi(temp->yoreName));
	}
	return;
}

Variable* scanVDCL(Node* node)
{
	if(EQ(node->attr,"int")) return makeVar(node->son->attr, false, _para, node->linenum, 0);
	else
	{
		if(node->son->sib) return makeVar(node->son->attr, true, _para, node->linenum, atoi(node->son->sib->attr));
		else return makeVar(node->son->attr, true, _para, node->linenum, 0);
	}
}

void addFunc(const char* funcName, Node* node, int declaring)
{
	int declaredBefore = false;
	int index;
	int i;
	for(i = 0; i < FuncCnt; ++i)
		if(EQ(funcName,FuncList[i]->funcName))
		{
			if(declaring || ((!declaring) && FuncList[i]->declared == false))
			{
				fprintf(stderr, "Line %d: function redefine or redeclare: %s\n", node->linenum, funcName);
				exit(1);
			}
			declaredBefore = true;
			index = i;
			break;
		}

	if(!declaredBefore)
	{
		Func* newFunc = (Func*)malloc(sizeof(Func));
		newFunc->funcName = strdup(funcName);
		newFunc->linenum = node->linenum;
		newFunc->declared = false;
		ParamCnt = 0;
		if(!(node->sib && node->sib->type == _vdcl))
			newFunc->first = NULL;
		else
		{
			newFunc->first = (parameter*)malloc(sizeof(parameter));
			node = node->sib;
			Variable* par = scanVDCL(node);
			parameter* tmp = newFunc->first;
			tmp->isarray = par->isarray;
			tmp->arraysize = par->arraysize;
			tmp->next = NULL;
			while(node->sib && node->sib->type == _vdcl)
			{
				tmp->next = (parameter*)malloc(sizeof(parameter));
				tmp = tmp->next;
				node = node->sib;
				Variable* par = scanVDCL(node);
				tmp->isarray = par->isarray;
				tmp->arraysize = par->arraysize;
				tmp->next = NULL;
			}
		}
		FuncList[FuncCnt++] = newFunc;
	}
	else
	{
		parameter* ppointer = FuncList[index]->first; 
		FuncList[index]->declared = false;
		while(node->sib && node->sib->type == _vdcl)
		{
			node = node->sib;
			Variable* par = scanVDCL(node);
			if(ppointer == NULL || ppointer->isarray != par->isarray || ppointer->arraysize != par->arraysize)
			{
				fprintf(stderr, "Line %d: function parameter type error: %s\n", node->linenum, funcName);
				exit(1);
			}
			ppointer = ppointer->next;
		}
	}
	return;
}

void scanFDEF(Node* node)
{
	if(indent)
	{
		fprintf(stderr, "Line %d: form error\n", node->linenum);
		exit(1);
	}

	if(node->type == _main)
	{
		fprintf(stdout, "f_main [0]\n");
		indent = true;
		node = node->son;

		while(node)
		{
			scanNode(node);
			node = node->sib;
		}

		indent = false;
		fprintf(stdout, "end f_main\n");
		return;
	}

	node = node->son;

	const char* funcName = node->attr;
	addFunc(funcName, node, false);

	fprintf(stdout, "f_%s [%d]\n", funcName, ParamCnt);
	indent = true;

	while(node->sib && node->sib->type == _vdcl) node = node->sib;

	while(node->sib)
	{
		node = node->sib;
		scanNode(node);
	}

	indent = false;
	fprintf(stdout, "end f_%s\n", funcName);
	return;
}

void scanFDCL(Node* node)
{
	node = node->son;
	if(indent) return;
	addFunc(node->attr, node, true);
	return;
}

void scanNode(Node* node)
{
	switch(node->type)
	{
		case _vdcl: scanVDCL(node); break;
		case _fdcl: scanFDCL(node); break;
		case _vdef: scanVDEF(node); break;
		case _fdef:
		case _main: scanFDEF(node); break;
		case _stat: scanSTAT(node); break;
		case _expr:
		case _iden:
		case _imme: scanEXPR(node); break; 
		default:
			fprintf(stderr, "Line %d: statement error: %s %d\n", node->linenum, node->attr, node->type);
			exit(1);
	}
}

void scanTree(Node* root)
{
	initEnv();
	while(root)
	{
		scanNode(root);
		root = root->sib;
	}
}
