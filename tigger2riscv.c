/*
	Programmer: Hugo Zhang
	Student ID: 1600012945
	Date:	    2018/12/17
*/

#include "tigger2riscv.h"
#define EQ(a,b) (strcmp(a,b)==0)
#define CPY(a, b, c) memcpy((void*)a, (const void*)b, (size_t)c)
#define CMP(a, b, c) memcmp((const void*)a, (const void*)b, (unsigned int)c)

Expr* ExprList[1000];
int stk = 0;
void makeEXPR(enum _expr_type type, const char* op, const char* src1, const char* src2, const char* dst)
{
	int i = exprIndex;
	++exprIndex;
	ExprList[i] = (Expr*)malloc(sizeof(Expr));
	ExprList[i]->index = i;
	ExprList[i]->type = type;
	ExprList[i]->op = op;
	ExprList[i]->src1 = src1;
	ExprList[i]->src2 = src2;
	ExprList[i]->dst = dst;
	return;
}

void printFunc(int index)
{
	stk = (atoi(ExprList[index]->src2)/4+1)*16;
	//fprintf(stdout, "    .size    f,.-f\n");
	fprintf(stdout, "    .text\n");
	fprintf(stdout, "    .align   2\n");
	fprintf(stdout, "    .global  %s\n", ExprList[index]->op);
	fprintf(stdout, "    .type    %s,@function\n", ExprList[index]->op);
	fprintf(stdout, "%s:\n", ExprList[index]->op);
	fprintf(stdout, "    add      sp,sp,-%d\n", stk);
	fprintf(stdout, "    sw       ra,%d(sp)\n", stk-4);
	return;
}

void printDecl(int index)
{
	int size = atoi(ExprList[index]->src1);
	const char * name = ExprList[index]->op;
	fprintf(stdout, "    .comm    %s,%d,4\n", name, size*4);
	return;
}

void printOpe2(int index)
{
	const char* op = ExprList[index]->op;
	const char* src1 = ExprList[index]->src1;
	const char* src2 = ExprList[index]->src2;
	const char* dst = ExprList[index]->dst;
	if(EQ(op,"+"))
		fprintf(stdout, "    add      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"-"))
		fprintf(stdout, "    sub      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"*"))
		fprintf(stdout, "    mul      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"/"))
		fprintf(stdout, "    div      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"%"))
		fprintf(stdout, "    rem      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"<"))
		fprintf(stdout, "    slt      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,">"))
		fprintf(stdout, "    sgt      %s,%s,%s\n", dst, src1, src2);
	else if(EQ(op,"&&"))
	{
		fprintf(stdout, "    and      %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    snez     %s,%s\n", dst, dst);
	}
	else if(EQ(op,"||"))
	{
		fprintf(stdout, "    or       %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    snez     %s,%s\n", dst, dst);
	}
	else if(EQ(op,"!="))
	{
		fprintf(stdout, "    xor      %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    snez     %s,%s\n", dst, dst);
	}
	else if(EQ(op,"=="))
	{
		fprintf(stdout, "    xor      %s,%s,%s\n", dst, src1, src2);
		fprintf(stdout, "    seqz     %s,%s\n", dst, dst);
	}
	return;
}

void printOpe1(int index)
{
	const char* op = ExprList[index]->op;
	const char* src = ExprList[index]->src1;
	const char* dst = ExprList[index]->dst;
	if(EQ(op,"!"))
		fprintf(stdout, "    seqz     %s,%s\n", dst, src);
	else fprintf(stdout, "    sub      %s,x0,%s\n", dst, src);
	return;
}

void printAssi(int index)
{
	const char* left = ExprList[index]->op;
	const char* right = ExprList[index]->src1;
	const char* larray = ExprList[index]->src2;
	const char* rarray = ExprList[index]->dst;
	if(rarray != NULL)
		fprintf(stdout, "    lw       %s,%s(%s)\n", left, rarray, right);
	else if(larray != NULL)
		fprintf(stdout, "    sw       %s,%s(%s)\n", right, larray, left);
	else if(left[0] == 'v')
	{
		fprintf(stdout, "    .global  %s\n", left);
		fprintf(stdout, "    .section .sdata\n");
		fprintf(stdout, "    .align   2\n");
		fprintf(stdout, "    .type    %s,@object\n", left);
		fprintf(stdout, "    .size    %s,4\n", left);
		fprintf(stdout, "%s:\n", left);
		fprintf(stdout, "    .word    0\n");
	}
	else if(isdigit(right[0]))
		fprintf(stdout, "    li       %s,%s\n", left, right);
	else fprintf(stdout, "    mv       %s,%s\n", left, right);
	return;
}

void printIfbr(int index)
{
	const char* op = ExprList[index]->op;
	const char* src1 = ExprList[index]->src1;
	const char* src2 = ExprList[index]->src2;
	const char* label = ExprList[index]->dst;
	if(EQ(op,"!="))
		fprintf(stdout, "    bne");
	else if(EQ(op,"=="))
		fprintf(stdout, "    beq");
	else if(EQ(op,"<"))
		fprintf(stdout, "    blt");
	else fprintf(stdout, "    bgt");
	fprintf(stdout, "      %s,%s,.l%s\n", src1, src2, label);
	return;
}

void printGoto(int index)
{
	fprintf(stdout, "    j        .l%s\n", ExprList[index]->op);
	return;
}

void printPlab(int index)
{
	fprintf(stdout, ".l%s:\n", ExprList[index]->op);
	return;
}

void printCall(int index)
{
	fprintf(stdout, "    call     %s\n", ExprList[index]->op);
	return;
}

void printStor(int index)
{
	int var = atoi(ExprList[index]->src1);
	const char* reg = ExprList[index]->op;
	fprintf(stdout, "    sw       %s,%d(sp)\n", reg, var*4);
	return;
}

void printLoad(int index)
{
	const char* var = ExprList[index]->op;
	const char* reg = ExprList[index]->src1;
	if(isdigit(var[0]))
		fprintf(stdout, "    lw       %s,%d(sp)\n", reg, atoi(var)*4);
	else
	{
		fprintf(stdout, "    lui      %s,%%hi(%s)\n", reg, var);
		fprintf(stdout, "    lw       %s,%%lo(%s)(%s)\n", reg, var, reg);
	}
	return;
}

void printLdad(int index)
{
	const char* var = ExprList[index]->op;
	const char* reg = ExprList[index]->src1;
	if(isdigit(var[0]))
		fprintf(stdout, "    add      %s,sp,%d\n", reg, atoi(var)*4);
	else
	{
		fprintf(stdout, "    lui      %s,%%hi(%s)\n", reg, var);
		fprintf(stdout, "    add      %s,%s,%%lo(%s)\n", reg, reg, var);
	}
	return;
}

void printRetu(int index)
{
	fprintf(stdout, "    lw       ra,%d(sp)\n", stk-4);
	fprintf(stdout, "    add      sp,sp,%d\n", stk);
	fprintf(stdout, "    jr       ra\n");
	return;
}

void printEndf(int index)
{
	fprintf(stdout, "    .size    %s,.-%s\n", ExprList[index]->op, ExprList[index]->op);
	return;
}

void translate()
{
	int i = 0;
	for(; i < exprIndex; ++i)
	{
		switch(ExprList[i]->type)
		{
			case _func: printFunc(i); break;
			case _decl: printDecl(i); break;
			case _ope2: printOpe2(i); break;
			case _ope1: printOpe1(i); break;
			case _assi: printAssi(i); break;
			case _ifbr: printIfbr(i); break;
			case _goto: printGoto(i); break;
			case _plab: printPlab(i); break;
			case _call: printCall(i); break;
			case _stor: printStor(i); break;
			case _load: printLoad(i); break;
			case _ldad: printLdad(i); break;
			case _retu: printRetu(i); break;
			case _endf: printEndf(i); break;
		}
	}
	return;
}
