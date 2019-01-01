/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/1
*/

#include "parse2tigger.h"
#define EQ(a,b) (strcmp(a,b)==0)
#define CPY(a, b, c) memcpy((void*)a, (const void*)b, (size_t)c)
#define CMP(a, b, c) memcmp((const void*)a, (const void*)b, (unsigned int)c)

Expr* ExprList[1000];
Var* VarList[100];
ResEntry* ResList[1000];
short actTab[1000][100];
int TvarIndex[100];
int tvarIndex[100];
int labelIndex[200];
int gotoIndex[200];
int regUsage[19];	//t first, s next, positive means var, -1 means imme, -2 means invalid
int endtime[19];
int regIndex[100];	//positive means register, negative means stack, 20 means not valid
int funcSaveReg[19];


void analyzeExpr(Expr* expr);

void alterTab(int index, const char* src)
{
	char temp = src[0];
	if(temp == 'T') actTab[index][TvarIndex[atoi(src+1)]] = true;
	else if(temp == 't') actTab[index][tvarIndex[atoi(src+1)]] = true;
	return;
}

void alterTabF(int index, const char* src)
{
	if(index < 0) return;
	char temp = src[0];
	if(temp == 'T' && VarList[TvarIndex[atoi(src+1)]]->global==false) 
		actTab[index][TvarIndex[atoi(src+1)]] = false;
	else if(temp == 't') actTab[index][tvarIndex[atoi(src+1)]] = false;
	return;
}

void analyzeDecl(Expr* expr)
{
	CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	int index = expr->index;
	char temp = (expr->op)[0];
	if(temp == 'T') actTab[index][TvarIndex[atoi(expr->op+1)]] = false;
	else if(temp == 't') actTab[index][tvarIndex[atoi(expr->op+1)]] = false;
	return;
}


void analyzeOp(Expr* expr)
{
	CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	int index = expr->index;
	alterTabF(index-1, expr->dst);
	alterTab(index, expr->src1);
	if(expr->src2 == NULL) return;
	alterTab(index, expr->src2);
	return;
}

void analyzeAssign(Expr* expr)
{
	CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	int index = expr->index;
	alterTabF(index-1, expr->src1);
	alterTab(index, expr->op);
	if(expr->dst != NULL) alterTab(index, expr->dst);
	return;
}

void analyzeIf(Expr* expr)
{
	CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	int i;
	int num = labelIndex[atoi(expr->dst)];
	for(i = 0; i < 100; ++i)
		if(actTab[num][i] == true) actTab[expr->index][i] = true;
	alterTab(expr->index, expr->src1);
	alterTab(expr->index, expr->src2);
	return;
}

void analyzeGoto(Expr* expr)
{
	int num = atoi(expr->op);
	CPY(actTab[expr->index], actTab[labelIndex[num]], sizeof(actTab[0]));
	return;
}

void analyzeLabel(Expr* expr)
{
	int num = atoi(expr->op);
	CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	if(gotoIndex[num] < expr->index) return;
	while(CMP(actTab[gotoIndex[num]], actTab[expr->index], sizeof(actTab[0])) < 0)
	{
		int i = gotoIndex[num];
		for(; i > expr->index; --i)
			analyzeExpr(ExprList[i]);
		CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	}
	return;
}

void analyzeOne(Expr* expr)
{
	if(expr->op == NULL) return;
	char temp = (expr->op)[0];
	CPY(actTab[expr->index], actTab[expr->index+1], sizeof(actTab[0]));
	if(isdigit(temp)) return;
	int num = atoi(expr->op+1);
	if(expr->type == _para) alterTab(expr->index, expr->op);
	else alterTabF(expr->index-1, expr->op);
	return;
}

void analyzeRet(Expr* expr)
{
	alterTab(expr->index, expr->op);
	return;
}

void analyzeExpr(Expr* expr)
{
	switch(expr->type)
	{
		case _func: break;
		case _decl: analyzeDecl(expr); break;
		case _ope2: analyzeOp(expr); break;
		case _ope1: analyzeOp(expr); break;
		case _assi: analyzeAssign(expr); break;
		case _ifbr: analyzeIf(expr); break;
		case _goto: analyzeGoto(expr); break;
		case _plab: analyzeLabel(expr); break;
		case _para: analyzeOne(expr); break;
		case _call: analyzeOne(expr); break;
		case _retu: analyzeRet(expr); break;
		default: break;
	}
	return;
}

void addVar(const char* varName, short global, short array, const char* size)
{
	int i = varIndex;
	++varIndex;
	VarList[i] = (Var*)malloc(sizeof(Var));
	VarList[i]->index = i;
	VarList[i]->name = varName;
	VarList[i]->global = global;
	VarList[i]->array = array;
	VarList[i]->size = (array?atoi(size):0);
	if(varName[0] == 'T') TvarIndex[atoi(varName+1)] = i;
	else tvarIndex[atoi(varName+1)] = i;
	return;
}

void makeEXPR(enum _expr_type type, const char* op, const char* src1, const char* src2, const char* dst)
{
	int i = exprIndex;
	if(i != 0 && type == _ifbr && EQ(ExprList[i-1]->op,op) && EQ(ExprList[i-1]->src1,src1) 
		&& EQ(ExprList[i-1]->src2,src2) && EQ(ExprList[i-1]->dst,dst))
		return;
	++exprIndex;
	ExprList[i] = (Expr*)malloc(sizeof(Expr));
	ExprList[i]->index = i;
	ExprList[i]->type = type;
	ExprList[i]->op = op;
	ExprList[i]->src1 = src1;
	ExprList[i]->src2 = src2;
	ExprList[i]->dst = dst;
	if(type == _plab) labelIndex[atoi(op)] = i;
	else if(type == _goto) gotoIndex[atoi(op)] = i;
	else if(type == _ifbr) gotoIndex[atoi(dst)] = i;
	return;
}

void updateTable()
{
	memset(actTab, 0, sizeof(actTab));
	int i;
	for(i = exprIndex-1; i >=0; --i)
		analyzeExpr(ExprList[i]);
	return;
}

void makeEntry(RTYPE type, int index, const char* op, const char* src1, const char* src2, const char* dst)
{
	ResList[index] = (ResEntry*)malloc(sizeof(ResEntry));
	ResList[index]->type = type;
	ResList[index]->index = index;
	ResList[index]->op = (op==NULL? NULL:strdup(op));
	ResList[index]->src1 = (src1==NULL?NULL:strdup(src1));
	ResList[index]->src2 = (src2==NULL?NULL:strdup(src2));
	ResList[index]->dst = (dst==NULL?NULL:strdup(dst));
	return;
}

const char* reg2string(int regnum)
{
	char temp[5] = {0};
	if(regnum <= 6)	sprintf(temp, "t%d", regnum);
	else sprintf(temp, "s%d", regnum-7);
	return strdup(temp);
}

int string2reg(const char* regstring)
{
	if(regstring[0] == 't') return atoi(regstring+1);
	else if(regstring[0] == 's') return (7+atoi(regstring+1));
	else return -1;
}

int suitableReg(int * pstackSize, int contra1, int contra2)
{
	int i = 0;
	int lastendind = -1;
	int lastendval = -1;
	for(i = 0; i < 19; ++i)
	{
		if(i == contra1 || i == contra2) continue;
		if(regUsage[i] < -1)
		{
			if(i > 6 && funcSaveReg[i] == -1)
			{
				int upd_i = resultIndex;
				++resultIndex;
				char sta[5] = {0};
				sprintf(sta, "%d", *pstackSize);
				makeEntry(_rstor, upd_i, reg2string(i), sta, NULL, NULL);
				funcSaveReg[i] = *pstackSize;
				++(*pstackSize);
			}
			return i;
		}
		if(regUsage[i] > -1 && endtime[i] >= lastendval)
		{
			lastendval = endtime[i];
			lastendind = i;
		}
	}

	const char* temp = reg2string(lastendind);
	char sta[5] = {0};
	sprintf(sta, "%d", *pstackSize);
	regIndex[regUsage[lastendind]] = -(*pstackSize);
	++(*pstackSize);
	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rstor, upd_i, temp, sta, NULL, NULL);
	return lastendind;
}

const char* turnToUse(int index, const char* varName, int* pstackSize, short leftside, int contra1, int contra2)
{
	char temp[5] = {0};
	if(varName[0] == 'p')
	{
		temp[0] = 'a';
		sprintf(temp+1, "%s", varName+1);
		return strdup(temp);
	}
	int varIndex;
	short imme = false;
	if(varName[0] == 't') varIndex = tvarIndex[atoi(varName+1)];
	else if(varName[0] == 'T') varIndex = TvarIndex[atoi(varName+1)];
	else imme = true;

	if(imme == false && regIndex[varIndex] >= 0 && regIndex[varIndex] < 20)
	{
		strcpy(temp, reg2string(regIndex[varIndex]));
		return strdup(temp);
	}

	int regnum = suitableReg(pstackSize, contra1, contra2);

	strcpy(temp, reg2string(regnum));
	if(imme == true)
	{
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rassi, upd_i, reg2string(regnum), varName, NULL, NULL);
		regUsage[regnum] = -1;
		return strdup(temp);
	}
	if(regIndex[varIndex] < 0 && leftside == false)
	{

		int upd_i = resultIndex;
		++resultIndex;
		char stackint[5] = {0};
		sprintf(stackint, "%d", -regIndex[varIndex]);
		makeEntry(_rload, upd_i, stackint, temp, NULL, NULL);
	}
	else if(VarList[varIndex]->array == true && leftside == false)
	{
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rldad, upd_i, VarList[varIndex]->globalname, temp, NULL, NULL);
	}
	if(imme == false)
	{
		regUsage[regnum] = varIndex;
		regIndex[varIndex] = regnum;
		int ending = index+1;
		while(ending < exprIndex && actTab[ending][varIndex] == true)
			++ending;
		endtime[regnum] = ending;
	}
	return strdup(temp);
}

const char* turnParamToUse(int index, const char* varName, int* pstackSize, int StackPlace)
{
	if(varName[0] != 'p') return turnToUse(index, varName, pstackSize, false, -1, -1);
	int temp = suitableReg(pstackSize, -1, -1);
	int upd_i = resultIndex;
	++resultIndex;
	char sta[5] = {0};
	sprintf(sta, "%d", StackPlace);
	makeEntry(_rload, upd_i, sta, reg2string(temp), NULL, NULL);
	return reg2string(temp);
}

void disableReg(int index)
{
	if(index == exprIndex-1) return;
	++index;
	int i = 0;
	for(; i < 19; ++i)
	{
		if(regUsage[i] >= 0 && actTab[index][regUsage[i]] == false)
		{
			if(regUsage[i] >= 0) regIndex[regUsage[i]] = 20;
			regUsage[i] = -2;
		}
		if(regUsage[i] == -1)
			regUsage[i] = -2;
	}
	return;
}

void changeGlobal(const char* leftvar, int* pstackSize)
{
	int varIndex;
	if(leftvar[0] == 'T') varIndex = TvarIndex[atoi(leftvar+1)];
	else return;
	if(VarList[varIndex]->global == false) return;
	if(VarList[varIndex]->array == true) return;
	const char* globalname = VarList[varIndex]->globalname;
	int reg0 = regIndex[varIndex];
	int reg1 = suitableReg(pstackSize, reg0, -1);
	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rldad, upd_i, globalname, reg2string(reg1), NULL, NULL);
	upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rassi, upd_i, reg2string(reg1), reg2string(reg0), "0", NULL);
	return;
}

void handleDecl(int index, int* pstackSize)
{
	if(ExprList[index]->dst == NULL) return;
	if(ExprList[index]->src1 != NULL) return;
	int varIndex;
	const char* varName = ExprList[index]->op;
	if(varName[0] != 'T') return;
	varIndex = TvarIndex[atoi(varName+1)];
	char sta[5] = {0};
	sprintf(sta, "%d", *pstackSize);
	VarList[varIndex]->globalname = sta;
	(*pstackSize) += (atoi(ExprList[index]->dst))/4;
	return;
}

void handleOpe2(int index, int* pstackSize)
{
	const char* src1 = turnToUse(index, ExprList[index]->src1, pstackSize, false, -1, -1);
	const char* src2 = turnToUse(index, ExprList[index]->src2, pstackSize, false, string2reg(src1), -1);
	const char* dst = turnToUse(index, ExprList[index]->dst, pstackSize, true, string2reg(src1), string2reg(src2));
	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rope2, upd_i, ExprList[index]->op, src1, src2, dst);
	changeGlobal(ExprList[index]->dst, pstackSize);
	disableReg(index);
	return;
}

void handleOpe1(int index, int* pstackSize)
{
	const char* src1 = turnToUse(index, ExprList[index]->src1, pstackSize, false, -1, -1);
	const char* dst = turnToUse(index, ExprList[index]->dst, pstackSize, true, string2reg(src1), -1);
	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rope1, upd_i, ExprList[index]->op, src1, NULL, dst);
	changeGlobal(ExprList[index]->dst, pstackSize);
	disableReg(index);
	return;
}

void handleAssi(int index, int* pstackSize)
{
	if(EQ(ExprList[index]->src2,"0"))
	{
		const char* src1 = turnToUse(index, ExprList[index]->op, pstackSize, false, -1, -1);
		const char* op = turnToUse(index, ExprList[index]->src1, pstackSize, true, string2reg(src1), -1);
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rassi, upd_i, op, src1, NULL, NULL);
		changeGlobal(ExprList[index]->src1, pstackSize);
		disableReg(index);
	}
	else if(EQ(ExprList[index]->src2,"1"))
	{
		const char* op = "+";
		const char* src1 = turnToUse(index, ExprList[index]->src1, pstackSize, false, -1, -1);
		const char* src2 = turnToUse(index, ExprList[index]->dst, pstackSize, false, string2reg(src1), -1);
		const char* dst = reg2string(suitableReg(pstackSize, string2reg(src1), string2reg(src2)));
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rope2, upd_i, op, src1, src2, dst);
		upd_i = resultIndex;
		++resultIndex;
		const char* temp = turnToUse(index, ExprList[index]->op, pstackSize, false, string2reg(dst), -1);
		makeEntry(_rassi, upd_i, dst, temp, "0", NULL);		
	}
	else
	{
		const char* op = "+";
		const char* src1 = turnToUse(index, ExprList[index]->op, pstackSize, false, -1, -1);
		const char* src2 = turnToUse(index, ExprList[index]->dst, pstackSize, false, string2reg(src1), -1);
		const char* dst = reg2string(suitableReg(pstackSize, string2reg(src1), string2reg(src2)));
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rope2, upd_i, op, src1, src2, dst);
		upd_i = resultIndex;
		++resultIndex;
		const char* temp = turnToUse(index, ExprList[index]->src1, pstackSize, false, string2reg(dst), -1);
		makeEntry(_rassi, upd_i, temp, dst, NULL, "0");
	}
}

void handleIfbr(int index, int* pstackSize)
{

	const char* src1 = turnToUse(index, ExprList[index]->src1, pstackSize, false, -1, -1);
	const char* src2 = turnToUse(index, ExprList[index]->src2, pstackSize, false, string2reg(src1), -1);

	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rifbr, upd_i, ExprList[index]->op , src1, src2, ExprList[index]->dst);
	return;
}

void handleGoto(int index, int *pstackSize)
{
	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rgoto, upd_i, ExprList[index]->op, NULL, NULL, NULL);
	return;
}

void handlePlab(int index, int *pstackSize)
{
	int label = atoi(ExprList[index]->op);
	if(gotoIndex[label]<index)
	{
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rplab, upd_i, ExprList[index]->op, NULL, NULL, NULL);
	}
	int i = 0;
	for(i = 0; i < varIndex; ++i)
	{
		if(actTab[index+1][i] == true && (regIndex[i] == 20 || regIndex[i] < 0))
		{
			regIndex[i] = suitableReg(pstackSize, -1, -1);
			regUsage[regIndex[i]] = i;
			if(VarList[i]->array == true)
			{
				int upd_i = resultIndex;
				++resultIndex;
				makeEntry(_rldad, upd_i, VarList[i]->globalname, reg2string(regIndex[i]), NULL, NULL);
			}
		}
	}
	if(gotoIndex[label]>index)
	{
		int upd_i = resultIndex;
		++resultIndex;
		makeEntry(_rplab, upd_i, ExprList[index]->op, NULL, NULL, NULL);
	}
	return;
}

void Translate(int pre_i)
{
	if(pre_i >= exprIndex) return;
	while(ExprList[pre_i]->type != _func && pre_i < exprIndex)
	{
		int upd_i = resultIndex;
		++resultIndex;
		char temp[5] = {0};
		temp[0] = 'v';
		sprintf(temp+1, "%d", globalIndex);
		++globalIndex;
		int varIndex = TvarIndex[atoi((ExprList[pre_i]->op)+1)];
		VarList[varIndex]->globalname = strdup(temp);
		if(ExprList[pre_i]->src2 == NULL) 
			makeEntry(_rdecl, upd_i, (const char*)temp, NULL, NULL, NULL);
		else 
			makeEntry(_rdecl, upd_i, (const char*)temp, ExprList[pre_i]->dst, NULL, NULL);
		++pre_i;
	}
	if (pre_i >= exprIndex) return;
	int i = 0, j;
	for(i = 0; i < 19; ++i) funcSaveReg[i] = -1;
	int stackSize = 0;
	int funcPara = atoi(ExprList[pre_i]->src1);
	int upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rfunc, upd_i, ExprList[pre_i]->op, ExprList[pre_i]->src1, NULL, NULL);
	++pre_i;
	int resultFunc = upd_i;
	int startStack;
	int paramNow = 0;

	while(ExprList[pre_i]->type != _ends)
	{
		switch(ExprList[pre_i]->type)
		{
			case _decl: handleDecl(pre_i++, &stackSize); break;
			case _ope2: handleOpe2(pre_i++, &stackSize); break;
			case _ope1: handleOpe1(pre_i++, &stackSize); break;
			case _assi: handleAssi(pre_i++, &stackSize); break;
			case _ifbr: handleIfbr(pre_i++, &stackSize); break;
			case _goto: handleGoto(pre_i++, &stackSize); break;
			case _plab: handlePlab(pre_i++, &stackSize); break;
			case _para:
			{
				if(paramNow == 0)
				{
					for(j = 0; j < 7; ++j)
						if(funcSaveReg[j] == -1 && regUsage[j] >= 0)
						{
							int upd_i = resultIndex;
							++resultIndex;
							funcSaveReg[j] = stackSize;
							++stackSize;
							char sta[5] = {0};
							sprintf(sta, "%d", funcSaveReg[j]);
							makeEntry(_rstor, upd_i, reg2string(j), sta, NULL, NULL);
						}
					startStack = stackSize;
					for(j = 0; j<funcPara; ++j)
					{
						int upd_i = resultIndex;
						++resultIndex;
						char temp1[5] = {0};
						temp1[0] = 'a';
						sprintf(temp1+1, "%d", j);
						char temp2[5] = {0};
						sprintf(temp2, "%d", stackSize++);
						makeEntry(_rstor, upd_i, temp1, temp2, NULL, NULL);
					}
					if(funcPara == 0)
					{
						int upd_i = resultIndex;
						++resultIndex;
						char temp1[5] = {0};
						temp1[0] = 'a';
						sprintf(temp1+1, "%d", 0);
						char temp2[5] = {0};
						sprintf(temp2, "%d", stackSize++);
						makeEntry(_rstor, upd_i, temp1, temp2, NULL, NULL);
					}
				}
				const char* src1 = turnParamToUse(pre_i, ExprList[pre_i]->op, &stackSize, startStack+paramNow);
				int upd_i = resultIndex;
				++resultIndex;
				char temp[5] = {0};
				temp[0] = 'a';
				sprintf(temp+1, "%d", paramNow);
				makeEntry(_rassi, upd_i, temp, src1, NULL, NULL);
				++paramNow;
				++pre_i;
				break;
			}
			case _call:
			{
				if(paramNow == 0)
				{
					for(j = 0; j < 7; ++j)
						if(funcSaveReg[j] == -1 && regUsage[j] >= 0)
						{
							int upd_i = resultIndex;
							++resultIndex;
							funcSaveReg[j] = stackSize;
							++stackSize;
							char sta[5] = {0};
							sprintf(sta, "%d", funcSaveReg[j]);
							makeEntry(_rstor, upd_i, reg2string(j), sta, NULL, NULL);
						}
					startStack = stackSize;
					int upd_i = resultIndex;
					++resultIndex;
					char temp1[5] = {0};
					temp1[0] = 'a';
					sprintf(temp1+1, "%d", 0);
					char temp2[5] = {0};
					sprintf(temp2, "%d", stackSize++);
					makeEntry(_rstor, upd_i, temp1, temp2, NULL, NULL);
				}
				int upd_i = resultIndex;
				++resultIndex;
				makeEntry(_rcall, upd_i, ExprList[pre_i]->src1, NULL, NULL, NULL);
				if(ExprList[pre_i]->op != NULL)
				{
					const char* op = turnToUse(pre_i, ExprList[pre_i]->op, &stackSize, true, -1, -1);
					upd_i = resultIndex;
					resultIndex++;
					makeEntry(_rassi, upd_i, op, "a0", NULL, NULL);
					changeGlobal(ExprList[pre_i]->op, &stackSize);
				}
				disableReg(pre_i);
				for(i = 0; i < funcPara; ++i)
				{
					int upd_i = resultIndex;
					++resultIndex;
					char temp1[5] = {0};
					sprintf(temp1, "%d", startStack+i);
					char temp2[5] = {0};
					temp2[0] = 'a';
					sprintf(temp2+1, "%d", i);
					makeEntry(_rload, upd_i, temp1, temp2, NULL, NULL);
				}
				if(paramNow == 0)
				{
					int upd_i = resultIndex;
					++resultIndex;
					char temp1[5] = {0};
					sprintf(temp1, "%d", startStack);
					char temp2[5] = "a0";
					makeEntry(_rload, upd_i, temp1, temp2, NULL, NULL);
				}
				else if(funcPara == 0)
				{
					int upd_i = resultIndex;
					++resultIndex;
					char temp1[5] = {0};
					sprintf(temp1, "%d", startStack);
					char temp2[5] = "a0";
					makeEntry(_rload, upd_i, temp1, temp2, NULL, NULL);	
				}
				for(i = 0; i < 7; ++i)
				{
					if(funcSaveReg[i] != -1)
					{
						char sta[5] = {0};
						sprintf(sta, "%d", funcSaveReg[i]);
						int upd_i = resultIndex;
						++resultIndex;
						makeEntry(_rload, upd_i, sta, reg2string(i), NULL, NULL);
						funcSaveReg[i] = -1;
					}
				}
				++pre_i;
				paramNow = 0;
				break;
			}
			case _retu:
			{
				for(i = 7; i < 19; ++i)
				{
					if(funcSaveReg[i] != -1)
					{
						int upd_i = resultIndex;
						++resultIndex;
						char sta[5] = {0};
						sprintf(sta, "%d", funcSaveReg[i]);
						makeEntry(_rload, upd_i, sta, reg2string(i), NULL, NULL);
					}
				}
				const char* src1 = turnToUse(pre_i, ExprList[pre_i]->op, &stackSize, false, -1, -1);
				upd_i = resultIndex;
				++resultIndex;
				makeEntry(_rassi, upd_i, "a0", src1, NULL, NULL);
				upd_i = resultIndex;
				++resultIndex;
				makeEntry(_rretu, upd_i, NULL, NULL, NULL, NULL);
				++pre_i;
			}
		}
	}


	upd_i = resultIndex;
	++resultIndex;
	makeEntry(_rends, upd_i, ExprList[pre_i]->op, NULL, NULL, NULL);
	++pre_i;
	char temp[5] = {0};
	sprintf(temp, "%d", stackSize);
	ResList[resultFunc]->src2 = strdup(temp);
	Translate(pre_i);
	return;
}

void printResult()
{
	int i = 0;
	for(; i < 19; ++i) regUsage[i] = -2;
	for(i = 0; i < 100; ++i) regIndex[i] = 20;


	Translate(0);

	for(i = 0; i < resultIndex; ++i)
	{
		switch(ResList[i]->type)
		{
			case _rfunc:
				fprintf(stdout, "f_%s [%s] [%s]\n", ResList[i]->op, ResList[i]->src1, ResList[i]->src2);
				break;
			case _rdecl:
				if(ResList[i]->src1 == NULL) fprintf(stdout, "%s = 0\n", ResList[i]->op);
				else fprintf(stdout, "%s = malloc %s\n", ResList[i]->op, ResList[i]->src1);
				break;
			case _rope2:
				fprintf(stdout, "    %s = %s %s %s\n", ResList[i]->dst, ResList[i]->src1, ResList[i]->op, ResList[i]->src2);
				break;
			case _rope1:
				fprintf(stdout, "    %s = %s %s\n", ResList[i]->dst, ResList[i]->op, ResList[i]->src1);
				break;
			case _rassi:
				if(ResList[i]->src2 == NULL && ResList[i]->dst == NULL)
					fprintf(stdout, "    %s = %s\n", ResList[i]->op, ResList[i]->src1);
				else if(ResList[i]->src2 != NULL)
					fprintf(stdout, "    %s[%s] = %s\n", ResList[i]->op, ResList[i]->src2, ResList[i]->src1);
				else fprintf(stdout, "    %s = %s[%s]\n", ResList[i]->op, ResList[i]->src1, ResList[i]->dst);
				break;
			case _rifbr:
				fprintf(stdout, "    if %s %s %s goto l%s\n", ResList[i]->src1, ResList[i]->op, ResList[i]->src2, ResList[i]->dst);
				break;
			case _rgoto:
				fprintf(stdout, "    goto l%s\n", ResList[i]->op);
				break;
			case _rplab:
				fprintf(stdout, "l%s:\n", ResList[i]->op);
				break;
			case _rcall:
				fprintf(stdout, "    call f_%s\n", ResList[i]->op);
				break;
			case _rstor:
				fprintf(stdout, "    store %s %s\n", ResList[i]->op, ResList[i]->src1);
				break;
			case _rload:
				fprintf(stdout, "    load %s %s\n", ResList[i]->op, ResList[i]->src1);
				break;
			case _rldad:
				fprintf(stdout, "    loadaddr %s %s\n", ResList[i]->op, ResList[i]->src1);
				break;
			case _rretu:
				fprintf(stdout, "    return\n");
				break;
			case _rends:
				fprintf(stdout, "end f_%s\n", ResList[i]->op);
				break;
		}
	}
	return;
}


void testExprList()
{
	int i = 0;
	for(; i < exprIndex; ++i)
	{
		fprintf(stdout, "%s\n", ExprList[i]->op);
	}
}

void testActtab()
{
	int v = varIndex;
	int e = exprIndex;
	int i, j;
	for(i = 0; i < v; ++i)
	{
		fprintf(stdout, "%s: ", VarList[i]->name);
		for(j = 0; j < e; ++j)
		{
			fprintf(stdout, " %d", actTab[j][i]);
		}
		fprintf(stdout, "\n");
	}
}