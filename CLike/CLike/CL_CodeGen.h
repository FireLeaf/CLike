/*******************************************************************************
FILE:		CL_CodeGen.h

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/27

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef _CL_CODEGEN_H_
#define _CL_CODEGEN_H_

#include <stack>

#include "CL_Syntax.h"

class Coff;

// @Struct operand item
struct Operand 
{
	Type type;				// data type
	unsigned short r;		// register or storage class
	int value;				// constant value
	Symbol* sym;			// related symbol
};

//@Class operand stack
class OperandStack
{
public:
	// @Function operand in stack
	OperandStack(){}
	~OperandStack(){
		for (int i = 0; i < stack_operand.size() i++)
		{

		}
	}

	// @Function push operand
	// @Param type operand data type
	// @Param r operand storage class
	// @Param value operand value
	void push(Type *type, int r, int value)
	{
		Operand* new_operand = new Operand;
		new_operand->type = *type;
		new_operand->r = r;
		new_operand->value = value;
		stack_operand.push_back(new_operand);
	}

	// @Function pop operand
	void pop()
	{
		if (0 == stack_operand.size())
		{
			error("stack empty");
			return;
		}
		Operand* tmp = stack_operand[stack_operand.size() - 1];
		stack_operand.erase(stack_operand.begin() + stack_operand.size() - 1);
		if (tmp)
			delete tmp;
	}

	// @Function swap stack top two element
	void swap()
	{
		Operand* tmp = stack_operand[stack_operand.size() - 1];
		stack_operand[stack_operand.size() - 1] = stack_operand[stack_operand.size() - 2];
		stack_operand[stack_operand.size() - 1] = tmp;
	}

	// @Function get element
	// @Param index the element index
	Operand* get(int index)
	{
		int idx = stack_operand.size() - 1 + index;
		if (idx >= 0 && idx < stack_operand.size())
			return stack_operand[idx];

		return NULL;
	}

	// @Function get stack top
	Operand* top() { return get(0); }

	// @Function get second stack top
	Operand* second_top() { return get(-1); }

	// @Function stack size
	int count() { return stack_operand.size(); }
public:
	std::vector < Operand* > stack_operand;
};

// @Class code gen
class CodeGen
{
public:
	CodeGen() : ptr_coff(NULL) {}
protected:
	// @Function push operand
	// @Param type operand data type
	// @Param r operand storage class
	// @Param value operand value
	void operand_push(Type *type, int r, int value);

	// @Function swap stack top two element
	void operand_swap();

	// @Function pop operand
	void operand_pop() { operand_stack.pop(); }

	// @Function assign operand
	// @Param opd the operand
	// @Param t the operand data type
	// @Param r the operand storage class
	// @Param value the operand value
	void operand_assign(Operand* opd, int t, int r, int value);

	// @Function check stack top operand whether is lvalue
	void check_lvalue();

	// @Function cancel stack top operand lvalue attribute
	//			 i.e. get stack top operand address, because
	//			 after generate code judge data or address
	//			 by lvalue attribute
	void cancel_lvalue();
public:
	// @Function generate a byte to code section
	// @Param c the byte
	void gen_byte(char c);

	// @Function generate instruction prefix
	// @Param opcode the opcode
	void gen_prefix(char opcode);

	// @Function generate single byte instruction
	// @Param opcode the opcode
	void gen_opcode1(char opcode);

	// @Function generate two byte instruction
	// @Param first the first byte
	// @Param second the second byte
	void gen_opcode2(char first, char second);

	// @Function generate instruction addressing mode byte ModR/M
	// @Param mod ModR/M[7:6] two bit
	// @Param reg_opcode ModR/M[5:3] three bit
	// @Param r_m ModR/M[2:0] three bit
	// @Param sym symbol pointer
	// @Param c symbol related value
	void gen_modrm(int mod, int reg_opcode, int r_m, Symbol* sym, int c);

	// @Function generate four byte instruction
	// @Param c four byte operand
	void gen_dword(unsigned int c);

	// @Function generate global address symbol, and add to relocation section
	// @Param r symbol storage class
	// @Param sym symbol pointer
	// @Param c the symbol related value
	void gen_addr32(int r, Symbol* sym, int c);

public:
	// @Function load operand to register r(eax,ecx,edx,ebx,....)
	// @Param r register index
	// @Param opd operand pointer
	void load(int r, Operand *opd);

	// @Function store register value to operand
	// @Param r register index
	// @Param opd operand pointer
	void store(int r, Operand* opd);

	// @Function load operand to 'rc' like register
	// @Return register index
	// @Param rc register type
	// @Param opd operand pointer
	int load_1(int rc, Operand* opd);

	// @Function load stack top operand to 'rc1' like register
	//			 load second stack top operand to 'rc2' like register
	// @Param rc1 register type what load stack top operand to register
	// @Param rc2 register type what load second stack top operand to register
	void load_2(int rc1, int rc2);

	// @Function store stack top operand to stack second top operand
	void store0_1();

	// @Function record pending jump address
	// @Param s forward jump instruction address
	void makelist(int s);

	// @Function indirect addressing
	void indirection();
public:
	// @Function generate binary operator
	// @Param op operate type
	void gen_op(int op);

	// @Function generate integer operator
	// @Param op operate type
	void gen_opi(int op);

	// @Function generate integer binary operator
	// @Param opc ModR/M [5:3]
	// @Parm op operate type
	void gen_opi2(int opc, int op);

	// @Function generate jump to high address instruction, address pending
	// @Return ...
	// @Param t high instruction address
	int gen_jmpforward(int t);

	// @Function generate jump to low address instruction, address determine
	// @Param a low address
	void gen_jmpbackward(int a);

	// @Function generate condition jump instruction
	// @Return new jump instruction
	// @Param pre jump instruction address
	int gen_jcc(int t);

	// @Function generate function call, first push all parameters, then generate call code
	// @Param nb_args parameter number
	void gen_invoke(int nb_args);

	// @Function generate call instruction
	void gen_call();
public:
	// @Function allocate register
	// @Param rc register type
	int allocate_reg(int rc);

	// @Function put register 'r' to memory stack, and mark release the operand which in 'r' register  to local variable
	// @Param r register index
	void spill_reg(int r);

	// @Function overflow all reigster to momory stack
	void spill_regs();
protected:
	// @Property operand stack
	OperandStack operand_stack;

	// @Property current instruction byte count
	int ind;

	// @Property record return instruction ip
	int rsym;

	// @Property local variable address in stack
	int loc;

	// @Property function begin instruction
	int func_begin_ind;

	// @Property function return need release space size
	int func_ret_sub;

	// @Property read only sction symbol
	Symbol * sym_sec_rdata;

	// @Property Coff class
	Coff* ptr_coff;
};

#endif