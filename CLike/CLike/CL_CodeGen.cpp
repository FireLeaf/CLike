/*******************************************************************************
FILE:		CL_CodeGen.cpp

DESCRIPTTION:

CREATED BY: YangCao, 2016/03/27

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include "CL_CodeGen.h"

void CodeGen::operand_push(Type *type, int r, int value)
{
	// check stack capacity
	//...
	operand_stack.push(type, r, value);
}

void CodeGen::operand_swap()
{
	operand_stack.swap();
}

void CodeGen::operand_assign(Operand* opd, int t, int r, int value)
{
	assert(opd);
	opd->type.t = t;
	opd->r = r;
	opd->value = value;
}

void CodeGen::check_lvalue()
{
	Operand* top = operand_stack.top();
	if (!(top->r & SC_LVAL))
		expect("left value");
}

void CodeGen::cancel_lvalue()
{
	Operand* top = operand_stack.top();
	check_lvalue();
	top->r &= ~SC_LVAL;
}

void CodeGen::gen_byte(char c)
{
	int ind_tmp = ind + 1;
	// check code section capacity
	if (ind_tmp > ptr_coff->sec_text->data_allocated)
		ptr_coff->section_realloc(ptr_coff->sec_text, ind_tmp);
	
	ptr_coff->sec_text->data[ind] = c;
	ind = ind_tmp;
}

void CodeGen::gen_prefix(char opcode)
{
	gen_byte(opcode);
}

void CodeGen::gen_opcode1(char opcode)
{
	gen_byte(opcode);
}

void CodeGen::gen_opcode2(char first, char second)
{
	gen_byte(first);
	gen_byte(second);
}

void CodeGen::gen_modrm(int mod, int reg_opcode, int r_m, Symbol* sym, int c)
{
	mod <<= 6;
	reg_opcode <<= 3;

	if (mod == 0xc0)
	{
		// mod == 11 register addressing
		// 89 e5 (mod = 11 reg_code = 100->esp r = 101->ebp) mov ebp, esp
		gen_byte(mod | reg_opcode | (r_m & SC_VALMASK));
	}
	else if ( (r_m & SC_VALMASK) == SC_GLOBAL )
	{
		// mod == 00 r =101 direct memory addressing
		// 8b 05 50 30 40 00 mov eax, dword ptr[403050]
		// 0x05 indicate memory
		gen_byte(0x05 | reg_opcode);
		gen_addr32(r_m, sym, c);
	}
	else if ( (r_m & SC_VALMASK) == SC_LOCAL)
	{
		if (c == static_cast<char>(c))
		{
			// mod == 01 r = 101 disp8[ebp]
			// 89 45 fc mov dword ptr ss::[ebp - 4], eax
			gen_byte(0x45 | reg_opcode);
			gen_byte(c);
		}
		else
		{
			//mod == 10 r = 101 disp32[ebp]
			//89 85 a0fdffff mov dword ptr ss:[ebp - 260], eax
			gen_byte(0x85 | reg_opcode);
			gen_dword(c);
		}
	}
	else
	{
		// mod == 00
		// 89 0a (mod == 00 reg_opcode = 000->eax r = 001->ecx) mov dword ptr ds:[ecx], eax
		gen_byte(0x00 | reg_opcode | (r_m & SC_VALMASK));
	}
}

void CodeGen::gen_dword(unsigned int c)
{
	gen_byte(static_cast<char>(c));
	gen_byte(static_cast<char>(c >> 8));
	gen_byte(static_cast<char>(c >> 16));
	gen_byte(static_cast<char>(c >> 24));
}

void CodeGen::gen_addr32(int r, Symbol* sym, int c)
{
	if (r & SC_SYM)
		ptr_coff->coffreloc_add(ptr_coff->sec_text, sym, ind, IMAGE_REL_I386_DIR32);
	gen_dword(c);
}

void CodeGen::load(int r, Operand *opd)
{
	int v = 0, ft = 0, fc = 0, fr = 0;

	fr = opd->r;
	ft = opd->type.t;
	fc = opd->value;

	v = fr & SC_VALMASK;
	if (fr & SC_LVAL)
	{
		if ((ft & T_BTYPE) == T_CHAR)
		{
			// movsz -- move with sign-extention
			// 0f be /r movsx r32, r/m8 
			gen_opcode2(0x0f, 0xbe);
		}
		else if ( (ft & T_BTYPE) == T_SHORT )
		{
			// movsx
			// 0f bf /r movsx r32, r/m16
			gen_opcode2(0x0f, 0xbf);
		}
		else
		{
			// 8b /r mov r32,r/m32 mov r/m32 to r32
			gen_opcode1(0x8b);
		}
		gen_modrm(ADDR_OTHER, r, fr, opd->sym, fc);
	}
	else
	{
		if (v== SC_GLOBAL)
		{
			// b8 + rb mov r32, imm32
			gen_opcode1(0xb8 + r);
			gen_addr32(fr, opd->sym, fc);
		}
		else if (v == SC_LOCAL)
		{
			// 8d /r lea r32, m
			gen_opcode1(0x8d);
			gen_modrm(ADDR_OTHER, r, SC_LOCAL, opd->sym, fc);
		}
		else if (v == SC_CMP) // apply to c = a > b
		{
			/* c = a > b generate asm code
				39C8			CMP EAX, ECX
				B8 00000000		MOV EAX, 0
				0F9FC0			SETG AL
				8945 FC			MOV DWORD PTR SS:[EBP - 4], EAX
			*/
			// B8 + rd mov r 32, imm32
			gen_opcode1(0xb8 + r);	// mov r, 0
			gen_dword(0);

			//0F 9F  SETG r/m8 set byte if greater (ZF = 0, and SF = OF)
			//0F 8F cw/cd JG rel16/32 jump near if greater(ZF = 0 and SF=OF)
			gen_opcode2(0x0f, fc + 16);
			gen_modrm(ADDR_REG, 0, r, NULL, 0);
		}
		else if (v != r)
		{
			// 89 /r MOV r/m32, r32
			gen_opcode1(0x89);
			gen_modrm(ADDR_REG, v, r, NULL, 0);
		}
	}
}

void CodeGen::store(int r, Operand* opd)
{
	int fr = 0, bt = 0;

	fr = opd->r & SC_VALMASK;
	bt = opd->type.t & T_BTYPE;
	if (bt == T_SHORT)
		gen_prefix(0x66);
	if (bt == T_CHAR)// 88 / r mov r/m, r8
		gen_opcode1(0x88);
	else // 89 /r mov r/m32, r32
		gen_opcode1(0x89);

	if (fr == SC_GLOBAL || fr == SC_LOCAL || (opd->r & SC_LVAL) )
		gen_modrm(ADDR_OTHER, r, opd->r, opd->sym, opd->value);
}

int CodeGen::load_1(int rc, Operand* opd)
{
	int r;
	r = opd->r & SC_VALMASK;

	if (r >= SC_GLOBAL || (opd->r & SC_LVAL))
	{
		r = allocate_reg(rc);
		load(r, opd);
	}
	opd->r = r;
	return r;
}

void CodeGen::load_2(int rc1, int rc2)
{
	load_1(rc2, operand_stack.top());
	load_1(rc1, operand_stack.second_top());
}

void CodeGen::store0_1()
{
	int r = 0, t = 0;

	r = load_1(REG_ANY, &operand_stack.top());

	// if lvalue overflow stack, need load to register
	Operand* second_top = operand_stack.second_top();
	if ((second_top->r & SC_VALMASK) == SC_LLOCAL)
	{
		Operand opd;
		t = allocate_reg(REG_ANY);
		operand_assign(&opd, T_INT, SC_LOCAL | SC_LVAL, second_top->value);
		load(t, &opd);
		second_top->r = t | SC_LVAL;
	}
	store(r, operand_stack.get(-2));
	operand_swap();
	operand_stack.pop();
}

void CodeGen::makelist(int s)
{
	int ind1 = 0;
	ind1 = ind + 4;

	if (ind1 > ptr_coff->sec_text->data_allocated)
		ptr_coff->section_realloc(ptr_coff->sec_text, ind1);

	*reinterpret_cast<int*>(ptr_coff->sec_text->data + ind) = s;
	s = ind;
	ind = ind1;
	return s;
}

void CodeGen::indirection()
{
	Operand *top = operand_stack.top();

	if ( (top->type.t & T_BTYPE) != T_PTR)
	{
		if ((top->type.t & T_BTYPE) == T_FUNC)
		{
			return;
		}
		expect("pointer");
	}

	if ((top->r & SC_LVAL))
		load_1(REG_ANY, top);
	top->type = *pointed_type(&top->type);

	// array and function is not left value
	if ( !(top->type.t & T_ARRAY)
		&& (top->type.t & T_BTYPE) != T_FUNC
		)
	{
		top->r |= SC_LVAL;
	}
}

void CodeGen::gen_op(int op)
{
	int u = 0, bt1 = 0, bt2 = 0;
	Type type1;

	Operand *top = operand_stack.top();
	Operand *second_top = operand_stack.second_top();

	bt1 = second_top->type.t & T_BTYPE;
	bt2 = top->type.t & T_BTYPE;

	if (bt1 == T_PTR || bt2 == T_PTR)
	{
		if (op >= TK_EQ && op < TK_GEQ)	// relation operator
		{
			gen_opi(op);
			top->type.t = T_INT;
		}
		else if (bt1 == T_PTR && bt2 == T_PTR)
		{
			if (op != TK_MINUS)
				error("two pointer only do relation or subtraction operator");
			u = pointed_size(&top->type);
			gen_opi(op);
			top->type.t = T_INT;
			operand_push(&int_type, SC_GLOBAL, u);
			gen_op(TK_DIVIDE);
		}
		else
		{
			if (op != TK_MINUS && op != TK_PLUS)
				error("pointer only do relation or minus or plus operator");

			// if the first pointer is operand
			if (bt2 == T_PTR)
				operand_swap();

			type1 = second_top->type;
			operand_push(&int_type, SC_GLOBAL, pointed_size(&second_top->type));
			gen_op(TK_STAR);

			gen_opi(op);
			top->type = type1;
		}
	}
	else
	{
		gen_opi(op);
		if (op >= TK_EQ && op < TK_GEQ)
		{
			// relation operator result is integer type
			top->type.t = T_INT;
		}
	}
}

void CodeGen::gen_opi(int op)
{
	int r = 0, fr = 0, opc = 0;

	switch (op)
	{
	case TK_PLUS:
		opc = 0;
		gen_opi2(opc, op);
		break;
	case TK_MINUS:
		opc = 5;
		gen_opi2(opc, op);
		break;
	case TK_STAR:
		{
			Operand *top = operand_stack.top();
			Operand *second_top = operand_stack.second_top();
			load_2(REG_ANY, REG_ANY);
			r = second_top->r;
			fr = top->r;
			operand_stack.pop();
			// mul sign
			gen_opcode2(0x0f, 0xaf);
			gen_modrm(ADDR_REG, r, fr, NULL, 0);
		}
		break;
	case TK_DIVIDE:
	case TK_MOD:
		{
			opc = 7;
			load_2(REG_EAX, REG_ECX);
			Operand *top = operand_stack.top();
			Operand *second_top = operand_stack.second_top();
			r = second_top->r;
			fr = top->r;
			operand_stack.pop();
			spill_reg(REG_EDX);

			// CWD, CDQ convert word to doubleword/convert doubleword to qradword
			// 99 CWQ EDX:EAX
			gen_opcode1(0x99);

			// IDIV 
			// F7 /7 IDIV r/m32
			// EDX:EAX / (r/m32)
			gen_opcode1(0xf7);
			gen_modrm(ADDR_REG, opc, fr, NULL, 0);

			if (op == TK_MOD)
				r = REG_EDX;
			else
				r = REG_EDX;

			top = operand_stack.top();
			top->r = r;
		}
		break;
	default:
		opc = 7;
		gen_opi2(opc, op);
		break;
	}
}

void CodeGen::gen_opi2(int opc, int op)
{
	int r = 0, fr = 0, c = 0;
	Operand *top = operand_stack.top();
	Operand *second_top = operand_stack.second_top();

	if ((top->r & (SC_VALMASK | SC_LVAL | SC_SYM)) == SC_GLOBAL)
	{
		r = load_1(REG_ANY, second_top);
		c = top->value;
		if (c == static_cast<char>(c))
		{
			// ADC Add with carry 83 /2 ib ADC r/m32, imm8
			// ADD Add			  83 /0 ib ADD r/m32, imm8
			// SUB Subtract		  83 /5 ib SUB r/m32, imm8
			// CMP Compare two op 83 /7 ib CMP r/m32, imm8
			gen_opcode1(0x83);
			gen_modrm(ADDR_REG, opc, r, NULL, 0);
			gen_byte(c);
		}
		else
		{
			// ADD add			 81 /0 id ADD r/m32, imm32
			// SUB Subtract		 81 /5 id SUB r/m32, imm32
			// CMP Compare		 81 /7 id CMP r/m32, imm32
			gen_opcode1(0x81);
			gen_modrm(ADDR_REG, opc, r, NULL, 0);
			gen_byte(c);
		}
	}
	else
	{
		load_2(REG_ANY, REG_ANY);
		r = second_top->r;
		fr = top->r;

		// ADD Add				 01 /r ADD r/m32, r32
		// SUB Subtract			 29 /r SUB r/m32, r32
		// CMP Compare			 39 /r CMP r/m32, r32
		gen_opcode1((opc << 3) | 0x01);
		gen_modrm(ADDR_REG, fr, r, NULL, 0);
	}

	operand_stack.pop();
	top = operand_stack.top();

	if (op >= TK_EQ && op <= TK_GEQ)
	{
		top->r = SC_CMP;
		switch (op)
		{
		case TK_EQ:
			top->value = 0x84;
			break;
		case TK_NEQ:
			top->value = 0x85;
			break;
		case TK_LT:
			top->value = 0x8c;
			break;
		case TK_LEQ:
			top->value = 0x8e;
			break;
		case TK_GT:
			top->value = 0x8f;
			break;
		case TK_GEQ:
			top->value = 0x8d;
			break;
		}
	}
}

int CodeGen::gen_jmpforward(int t)
{
	//jmp
	// e9 cd jmp
	gen_opcode1(0xe9);
	return makelist(t);
}

void CodeGen::gen_jmpbackward(int a)
{
	int r = 0;
	r = a - ind - 2;
	if (r == static_cast<char>(r))
	{
		//short jump		taget ip - current ip - 2
		// eb cb jmp rel8 jump short,relative,displacement relative to next instruction
		gen_opcode1(0xeb);
		gen_byte(r);
	}
	else
	{
		// long jump		target ip - current ip - 5
		// e9 cd jmp rel 32 
		gen_opcode1(0xe9);//insert a opcode then only minus four
		gen_dword(a - ind - 4);
	}
}

void CodeGen::gen_jcc(int t)
{
	int v = 0;
	int inv = 1;

	Operand *top = operand_stack.top();
	Operand *second_top = operand_stack.second_top();

	v = top->r & SC_VALMASK;

	if (v == SC_CMP)
	{
		// 0f 8f cw/cd				JG rel13/32 
		// ...
		gen_opcode2(0x0f, top->value ^ inv);
		t = makelist(t);
	}
	else
	{
		if ((top->r & (SC_VALMASK | SC_LVAL | SC_SYM)) == SC_GLOBAL)
			t = gen_jmpforward(t);
		else
		{
			v = load_1(REG_ANY, top);

			// TEST 
			// 85 /r TEST r/m32, r32
			gen_opcode1(0x85);
			gen_modrm(ADDR_REG, v, v, NULL, 0);

			// Jcc -- Jump if Condition Is Met
			// 0F 8F cw/cd JG rel16/32 near
			gen_opcode2(0x0f, 0x85 ^ inv);

			t = makelist(t);
		}
	}
	operand_stack.pop();
	return t;
}

void CodeGen::gen_invoke(int nb_args)
{
	int size = 0, r = 0, args_size = 0, func_call = 0;

	Operand *top = operand_stack.top();

	// push all parameter
	for (int i = 0; i < nb_args; i++)
	{
		r = load_1(REG_ANY, top);
		size = 4;

		// PUSH 
		// 50 +rd PUSH r32
		gen_opcode1(0x50 + r);
		args_size += size;
		operand_stack.pop();
	}
	
	top = operand_stack.top();

	spill_regs();
	func_call = top->type.ref->r; // get function call convention
	gen_call();
	if (args_size && func_call != KW_STDCALL)
		gen_addsp(args_size);
	operand_stack.pop();
}

void CodeGen::gen_call()
{
	Operand *top = operand_stack.top();
	int r = 0;

	if ( (top->r & (SC_VALMASK | SC_LVAL)) == SC_GLOBAL)
	{
		// record relocation information
		ptr_coff->coffreloc_add(ptr_coff->sec_text, top->sym, ind + 1, IMAGE_REL_I386_DIR32);

		// Call 
		// E8 CALL 32
		gen_opcode1(0xe8);
		gen_dword(top->value - 4);
	}
	else
	{
		// FF /2 CALL /rm32 call near, absolute indirect, address given in r/m32
		r = load_1(REG_ANY, top);
		gen_opcode1(0xff);				// call /jmp * r
		gen_opcode1(0xd0 + r);			// d0 = 11 010 000
	}
}

int CodeGen::allocate_reg(int rc)
{
	Operand *p = NULL;
	int used = 0;

	// find free register
	for (int r = 0; r <= REG_EBX; r++)
	{
		if (rc & REG_ANY || r == rc)
		{
			used = 0;
			for (int i = 0; i < operand_stack.count(); i++)
			{
				p = operand_stack.stack_operand[i];
				if ((p->r & SC_VALMASK) == r)
				{
					used = 1;
					break;// ......
				}
			}

			if (0 == used) return r;
		}
	}

	// if no free register, find the first used stack spill to memory stack
	for (int i = 0; i < operand_stack.count(); i++)
	{
		p = operand_stack.stack_operand[i];
		r = p->r & SC_VALMASK;
		if (r < SC_GLOBAL && (rc & REG_ANY || r == rc))
		{
			spill_reg(r);
			return r;
		}
	}

	return -1;
}

void CodeGen::spill_reg(int r)
{
	int size, align;

	Operand *p, opd;

	Type *type;

	for (int i = 0; i < operand_stack.count(); i++)
	{
		p = operand_stack.stack_operand[i];
		if ((p->r & SC_VALMASK) == r)
		{
			r = p->r & SC_VALMASK;
			type = &p->type;

			if (p->r & SC_LVAL)
				type = &int_type;
			size = type_size(type, &align);
			loc = calc_align(loc - size, align);
			operand_assign(&opd, type->t, SC_LOCAL | SC_LVAL, loc);
			store(r, &opd);
			if (p->r & SC_LVAL)
				p->r = (p->r & ~(SC_VALMASK)) | SC_LOCAL;
			else
				p->r = SC_LOCAL | SC_LVAL;
			p->value = loc;
			break;
		}
	}
}

void CodeGen::spill_regs()
{
	int r = 0;
	Operand *p = NULL;
	for (int i = 0; i < operand_stack.count(); i++)
	{
		p = operand_stack.stack_operand[i];
		r = p->r & SC_VALMASK;
		if (r < SC_GLOBAL)
			spill_reg(r);
	}
}