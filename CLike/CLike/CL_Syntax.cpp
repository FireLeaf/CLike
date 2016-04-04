/*******************************************************************************
FILE:		CL_Syntax.cpp

DESCRIPTTION:

CREATED BY: YangCao, 2015/05/13

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include "CL_Syntax.h"
#include "CL_Lex.h"
#include "CL_Util.h"
#include <assert.h>
using namespace Util;

Type char_pointer_type,		// 字符串指针				
	 int_type,				// int类型
	 default_func_type;		// 缺省函数类型

bool Syntax::type_specifier(Type* type)
{
	bool is_specifier = true;
	e_TypeCode t;
	Type type_struct;

	switch (token)
	{
	case KW_CHAR:
		t = T_CHAR;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_SHORT:
		t = T_SHORT;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_VOID:
		t = T_VOID;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_INT:
		t = T_INT;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_STRUCT:
		t = T_STRUCT;
		syntax_state = SNTX_SP;
		struct_specifier(&type_struct);
		type->ref = type_struct.ref;
		break;
	default:
		is_specifier = false;
		break;
	}
	
	if (is_specifier)
		type->t = t;
	return is_specifier;
}

bool Syntax::is_type_specifier(int _token)
{

	bool is_specifier = true;
	switch (_token)
	{
	case KW_CHAR:
	case KW_SHORT:
	case KW_VOID:
	case KW_INT:
	case KW_STRUCT:
		break;
	default:
		is_specifier = false;
		break;
	}
	return is_specifier;
}

int Syntax::init_syntax(const char* src_path)
{
	if ( -1 == lex.init_lex(src_path))
	{ 
		return -1;
	}

	coff.InitCoff(&lex, this);
	codegen.Init(&coff);

	int_type.t = T_INT;
	char_pointer_type.t = T_CHAR;
	mk_pointer(&char_pointer_type);
	default_func_type.t = T_FUNC;
	default_func_type.ref = sym_push(SC_ANOM, &int_type, KW_CDECL, 0);

	sym_sec_rdata = sec_sym_put(".rdata", 0);

	lex.get_char();
	lex.get_token();

	return 1;
}

void Syntax::translation_unit()
{
	while ( token != TK_EOF )
	{
		// in global space translation
		external_declatation(SC_GLOBAL);
	}
}

Symbol* Syntax::sym_direct_push(SymbolStack& ss, int v, Type* type, int c)
{
	Symbol *s = new Symbol;
	s->v = v;
	s->c = c;
	s->type = *type;
	s->next = NULL;
	s->prev_tok = NULL;
	ss.push(s);
	return s;
}

Symbol* Syntax::sym_push(int v, Type* type, int r, int c)
{
	Symbol *ps = NULL, **pps = NULL;
	TKWord *ts = NULL;
	
	if (!local_sym_stack.empty())
		ps = sym_direct_push(local_sym_stack, v, type, c);
	else
		ps = sym_direct_push(global_sym_stack, v, type, c);

	ps->r = r;

	if ( (v & SC_STRUCT) || v < SC_ANOM)
	{
		ts = lex.get_tkword(v & ~SC_STRUCT);
		if (v & SC_STRUCT)
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_identifier;
		ps->prev_tok = *pps;
		*pps = ps;
	}

	return ps;
}

Symbol* Syntax::func_sym_push(int v, Type* type)
{
	Symbol *s = NULL, **ps = NULL;
	s = sym_direct_push(global_sym_stack, v, type, 0);

	ps = &(lex.get_tkword(v)->sym_identifier);
	
	// insert the symbol to the same name symbol list end
	while (*ps)
		ps = &((*ps)->prev_tok);

	s->prev_tok = NULL;
	*ps = s;
	return s;
}

Symbol* Syntax::var_sym_put(Type* type, int r, int v, int addr)
{
	Symbol* sym = NULL;

	if ( (r & SC_VALMASK) == SC_LOCAL )
	{
		sym = sym_push(v, type, r, addr);
	}
	else if ( v && (r & SC_VALMASK) == SC_GLOBAL )
	{
		sym = sym_search(v);
		if (sym)
			error("%s redefinition\n", lex.get_tkword(v)->words);
		else
		{
			sym = sym_push(v, type, r | SC_SYM, 0);
		}
	}

	return sym;
}

Symbol* Syntax::sec_sym_put(const char* sec, int c)
{
	TKWord* tp;
	Symbol* s;
	Type type;
	type.t = T_INT;
	tp = lex.word_table.insert(sec);
	token = tp->tkcode;
	s = sym_push(token, &type, SC_GLOBAL, c);
	return s;
}

void Syntax::sym_pop(SymbolStack& ss, Symbol* b)
{
	Symbol *s, **ps;
	TKWord *ts;
	int v;

	s = ss.top();
	while ( s != b)
	{
		v = s->v;

		// if the pop symbol is struct or identifier, let symbol pointer the prev same name symbol
		if ( (v & SC_STRUCT) || v < SC_ANOM )
		{
			ts = lex.get_tkword(v & ~SC_STRUCT);
			if (v & SC_STRUCT)
				ps = &ts->sym_struct;
			else
				ps = &ts->sym_identifier;

			*ps = s->prev_tok;
		}

		ss.pop();
		delete s;// release symbol
		if (ss.empty())
			break;

		s = ss.top();
	}
}

Symbol* Syntax::struct_search(int v)
{
	if ( v >= static_cast<int>(lex.word_table.tk_wordtable.size()))
	{
		return NULL;
	}
	return lex.get_tkword(v)->sym_struct;
}

Symbol* Syntax::sym_search(int v)
{
	if (v >= static_cast<int>(lex.word_table.tk_wordtable.size()))
	{
		return NULL;
	}
	return lex.get_tkword(v)->sym_identifier;
}

void Syntax::external_declatation(e_StorageClass space)
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol* sym;
	Section* sec = NULL;

	if (!type_specifier(&btype))
	{
		expect("<specifer>");
	}

	// null statement
	if ( btype.t == T_STRUCT && token == TK_SEMICOLON)
	{
		lex.get_token();
		return;
	}

	while (true)
	{
		type = btype;
		declarator(&type, &v, NULL);

		if ( token == TK_BEGIN ) // parser the function
		{
			if (space == SC_LOCAL)
				error("nonsupport function nest function");
			
			if ((type.t & T_BTYPE) != T_FUNC)
				expect("<function definition>");

			sym = sym_search(v);
			if (sym)//函数前面声明过，现在是函数定义
			{
				if ((sym->type.t & T_BTYPE) != T_FUNC)
					error("'%s' redefine", lex.get_tkstr(v));
				sym->type = type;
			}
			else
				sym = func_sym_push(v, &type);
			funcbody(sym);
			break;
		}
		else
		{
			//函数声明
			if ( (type.t & T_BTYPE) == T_FUNC )
			{
				if (sym_search(v) == NULL)
				{
					sym = sym_push(v, &type, SC_GLOBAL | SC_SYM, 0);
				}
			}
			else //变量声明
			{
				r = 0;
				if ( !(type.t & T_ARRAY) )
					r |= SC_LVAL;
				
				r |= space;
				has_init = (token == TK_ASSIGN);

				if (has_init)//初值表达式
				{
					lex.get_token();
				}

				sec = allocate_storage(&type, r, has_init, v, &addr);
				sym = var_sym_put(&type, r, v, addr);
				if (space == SC_GLOBAL)
					coff.coffsym_add_update(sym, addr, sec->index, 0, IMAGE_SYM_CLASS_EXTERNAL);
				
				if (has_init)
				{
					initializer(&type, addr, sec);
				}
			}

			if (token == TK_COMMA)
			{
				lex.get_token();
			}
			else
			{
				syntax_state = SNTX_LF_HT;
				lex.skip(TK_SEMICOLON);
				break;
			}
		}
	}
}

void Syntax::struct_specifier(Type* type)
{
	lex.get_token();
	int v = token;
	Symbol *s;
	Type type_struct;
	
	syntax_state = SNTX_DELAY;
	lex.get_token();
	
	if (token == TK_BEGIN) // struct definition
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA) // sizeof (struct name)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP;

	lex.syntax_indent();

	if (v < TK_IDENT)
		expect("struct name is specifier");

	s = struct_search(v);
	if (!s)
	{
		type_struct.t = T_STRUCT;//....
		// -1赋值给s->c，表示该结构体尚未定义
		s = sym_push(v | SC_STRUCT, &type_struct, 0, -1);
		s->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = s;

	if (token == TK_BEGIN)
		struct_declaration_list(type);
}

void Syntax::struct_declaration_list(Type *type)
{
	int maxalign = 0, offset = 0;
	Symbol *s, **ps;
	s = type->ref;

	syntax_state = SNTX_LF_HT;	// the first struct member is different line with '{'
	syntax_level++;				// indent increase one

	lex.get_token();

	if (s->c != -1) //s->c记录结构体尺寸
		error("struct already definition");
	maxalign = 1;
	ps = &s->next;
	offset = 0;

	while (token != TK_END)
	{
		struct_declaration(&maxalign, &offset, &ps);
	}

	lex.skip(TK_END);// 

	s->c = calc_align(offset, maxalign);//calculate struct size
	s->r = maxalign;
	syntax_state = SNTX_LF_HT;
}

void Syntax::struct_declaration(int *maxalign, int *offset, Symbol***ps)
{
	int v, size, align;
	Symbol *ss;
	Type type_sub, btype;
	int force_align;

	//
	if (!type_specifier(&btype))
	{
		// To do here...
	}

	while (true)
	{
		v = 0;
		type_sub = btype;
		declarator(&type_sub, &v, &force_align);

		size = type_size(&type_sub, &align);

		if (force_align & ALIGN_SET)
			align = force_align & ~ALIGN_SET;

		*offset = calc_align(*offset, align);

		if (align > *maxalign)
			*maxalign = align;

		ss = sym_push(v | SC_MEMBER, &type_sub, 0, *offset);
		*offset += size;
		**ps = ss;
		*ps = &ss->next;

		if (token == TK_SEMICOLON || token == TK_EOF)
			break;
		lex.skip(TK_COMMA);
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::function_calling_convention(int* fc)
{
	assert(fc);

	if (token == KW_CDECL || token == KW_STDCALL)
	{
		*fc = token;
		syntax_state = SNTX_SP;
		lex.get_token();
	}
	else
		*fc = KW_CDECL;
}

void Syntax::struct_member_alignment(int * force_align)
{
	int align = 1;
	if (token == KW_ALIGN)
	{
		lex.get_token();
		lex.skip(TK_OPENPA);// "{"

		if (token == TK_CINT)
		{
			lex.get_token();
			align = tkvalue;
		}
		else
			expect("integer number");
		lex.skip(TK_CLOSEPA);//"}"

		if (align != 1 && align != 2 && align != 4)
			align = 1;
		align |= ALIGN_SET;
		*force_align = align;
	}
	else
		*force_align = 1;
}

void Syntax::declarator(Type *type, int *v, int *force_align)
{
	int fc;
	while ( token == TK_STAR)
	{
		mk_pointer(type);
		lex.get_token();
	}

	// check function call convention
	function_calling_convention(&fc);

	// check member alignment
	if (force_align)
		struct_member_alignment(force_align);

	direct_declarator(type, v, fc);
}

void Syntax::direct_declarator(Type* type, int *v, int func_call)
{
	if (token >= TK_IDENT)
	{
		*v = token;
		lex.get_token();
	}
	else
	{
		expect("identifier");
	}

	direct_declarator_postfix(type, func_call);
}

void Syntax::direct_declarator_postfix(Type* type, int func_call)
{
	int n;
	Symbol *s;

	if (token == TK_OPENPA)
	{
		parameter_type_list(type, func_call);
	}
	else if (token == TK_OPENBR)
	{
		lex.get_token();
		n = -1;
		if (token == TK_CINT)
		{
			lex.get_token();
			n = tkvalue;
		}
		lex.skip(TK_CLOSEBR);
		direct_declarator_postfix(type, func_call);
		s = sym_push(SC_ANOM, type, 0, n);
		type->t = T_ARRAY | T_PTR;
		type->ref = s;
	}
}

void Syntax::parameter_type_list(Type* type, int func_call)
{
	int n;
	Symbol **plast, *s, *first;
	Type pt;
	
	lex.get_token();
	first = NULL;
	plast = &first;

	while (token != TK_CLOSEPA)
	{
		if (!type_specifier(&pt))
			error("unknown type specifier");
		
		declarator(&pt, &n, NULL);
		s = sym_push(n | SC_PARAMS, &pt, 0, 0);
		*plast = s;
		plast = &s->next;
		if (token == TK_CLOSEPA)
			break;

		lex.skip(TK_COMMA);

		if (token == TK_ELLIPSIS)
		{
			func_call = KW_CDECL;
			lex.get_token();
			break;
		}
	}

	syntax_state = SNTX_DELAY;
	lex.skip(TK_CLOSEPA);

	//此处将函数返回类型存储，然后指向参数，最后type设为函数类型引用的相关信息放在ref中
	s = sym_push(SC_ANOM, type, func_call, 0);
	s->next = first;
	type->t = T_FUNC;
	type->ref = s;

	if (token == TK_BEGIN)
		syntax_state = SNTX_LF_HT;
	else
		syntax_state = SNTX_NUL;

	lex.syntax_indent();
}

void Syntax::funcbody(Symbol* sym)
{
	codegen.ind = coff.sec_text->data_offset;
	coff.coffsym_add_update(sym, codegen.ind, coff.sec_text->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);

	//放一匿名符号在局部符号表中
	sym_direct_push(local_sym_stack, SC_ANOM, &int_type, 0);

	gen_prolog(&sym->type);
	codegen.rsym = 0;
	compound_statement(NULL, NULL);//
	backpatch(codegen.rsym, codegen.ind);
	gen_epilog();
	coff.sec_text->data_offset = codegen.ind;
	//清空局部符号栈
	sym_pop(local_sym_stack, NULL);
}

void Syntax::gen_prolog(Type * func_type)
{
	int addr, align, size, func_call;
	int param_addr;
	Symbol *sym;
	Type *type;

	sym = func_type->ref;
	func_call = sym->r;
	addr = 8;
	codegen.loc = 0;
	codegen.func_begin_ind = codegen.ind;
	codegen.ind += FUNC_PROLOG_SIZE;
	// SUB ESP, ?? function parser end decided stack space and fill roll back
	if (sym->type.t == T_STRUCT)
		error("unsupported return struct");

	// parameters definition
	while ( (sym = sym->next) != NULL)
	{
		type = &sym->type;
		size = type_size(type, &align);
		size = calc_align(size, 4);	// push stack align to four byte

		// struct pass pointer
		if ( (type->t & T_BTYPE) == T_STRUCT )
		{
			size = 4;
		}

		param_addr = addr;
		addr += size;

		sym_push(sym->v & ~SC_PARAMS, type, SC_LOCAL | SC_LVAL, param_addr);

		codegen.func_ret_sub = 0;
		// __stdcall call convention, function clear stack by itself
		if (func_call == KW_STDCALL)
			codegen.func_ret_sub = addr - 8;
	}
}

void Syntax::gen_epilog()
{
	int v = 0, saved_ind = 0, opc = 0;

	// 8B /r mov r32, r/m32
	codegen.gen_opcode1(0x8b);				// mov esp, ebp
	codegen.gen_modrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0);

	// 58+ rd pop r32
	codegen.gen_opcode1(0x58 + REG_EBP);	// pop ebp

	if (codegen.func_ret_sub == 0)
	{
		// C3 ret
		codegen.gen_opcode1(0xc3);			// ret
	}
	else
	{
		// C2 iw	ret imm16
		codegen.gen_opcode1(0xc2);			// ret n
		codegen.gen_byte(static_cast<char>(codegen.func_ret_sub));
		codegen.gen_byte(static_cast<char>(codegen.func_ret_sub >> 8));
	}

	v = calc_align(-codegen.loc, 4);
	saved_ind = codegen.ind;
	codegen.ind = codegen.func_begin_ind;

	// PUSH
	// 50 + rd	push r32
	codegen.gen_opcode1(0x50 + REG_EBP);	// push ebp

	// 89 /r mov r/m32, r32
	codegen.gen_opcode1(0x89);
	codegen.gen_modrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0); // mov ebp, esp

	// SUB
	// 81 /5 id sub r/m32, imm32
	codegen.gen_opcode1(0x81);			// sub esp, stacksize
	opc = 5;
	codegen.gen_modrm(ADDR_REG, opc, REG_ESP, NULL, 0);
	codegen.gen_dword(v);

	codegen.ind = saved_ind;
}

void Syntax::initializer(Type* type, int c, Section* sec)
{
	if ( type->t & T_ARRAY && sec)
	{
		memcpy(sec->data + c, lex.tkstr.c_str(), lex.tkstr.length());
		lex.get_token();
	}
	else
	{
		assignment_expression();
		init_variable(type, sec, c, 0);
	}
}

void Syntax::init_variable(Type* type, Section* sec, int c, int v)
{
	int bt = 0;
	void * ptr;

	if (sec)
	{
		Operand* top = codegen.operand_stack.top();
		if ((top->r & (SC_VALMASK | SC_LVAL)) != SC_GLOBAL)
			error("global variable must initialize with constant");
		
		bt = type->t & T_BTYPE;
		ptr = sec->data + c;
		switch (bt)
		{
		case T_CHAR:
			*reinterpret_cast<char*>(ptr) = top->value;				// apply to char g_c = 'a';
			break;
		case T_SHORT:
			*reinterpret_cast<short*>(ptr) = top->value;			// apply to short g_s = 100;
			break;
		default:
			if (top->r & SC_SYM)
			{
				// apply to char * g_pstr = "hello";
				coff.coffreloc_add(sec, top->sym, c, IMAGE_REL_I386_DIR32);
			}
			*reinterpret_cast<int*>(ptr) = top->value;
			break;
		}
		codegen.operand_pop();
	}
	else
	{
		// initialize array 
		if (type->t & T_ARRAY)
		{
			codegen.operand_push(type, SC_LOCAL | SC_LVAL, c);
			codegen.operand_swap();
			codegen.spill_reg(REG_ECX);

			Operand* top = codegen.operand_stack.top();
			// B8 + rd mov r32, imm32
			codegen.gen_opcode1(0xB8 + REG_ECX);		// mov ecx, n
			codegen.gen_dword(top->type.ref->c);
			codegen.gen_opcode1(0xB8 + REG_ESI);
			codegen.gen_addr32(top->r, top->sym, top->value);	// mov esi, addr
			codegen.operand_swap();

			top = codegen.operand_stack.top();
			// Lea
			// 8D /r lea r32, m
			// lea edi, [ebp - n]
			codegen.gen_opcode1(0x8D);
			codegen.gen_modrm(ADDR_OTHER, REG_EDI, SC_LOCAL, top->sym, top->value);
			// Instruction prefix F3
			codegen.gen_prefix(0xf3);		// rep movs byte
			// MOVS/MOVSB/MOVSD
			// A4 MOVSB
			codegen.gen_opcode1(0xA4);

			codegen.operand_pop();
			codegen.operand_pop();
		}
		else
		{
			codegen.operand_push(type, SC_LOCAL | SC_LVAL, c);
			codegen.operand_swap();
			codegen.store0_1();
			codegen.operand_pop();
		}
	}
}

void Syntax::statement(int* bsym, int* csym)
{
	switch (token)
	{
	case TK_BEGIN:
		compound_statement(bsym, csym);
		break;
	case KW_IF:
		if_statement(bsym, csym);
		break;
	case KW_RETURN:
		return_statement();
		break;
	case KW_BREAK:
		break_statement(bsym);
		break;
	case KW_CONTINUE:
		continue_statement(csym);
		break;
	case KW_FOR:
		for_statement(bsym, csym);
		break;
	default:
		expression_statement();
		break;
	}
}

//复合语句
void Syntax::compound_statement(int *bsym, int *csym)
{
	Symbol *s;
	s = local_sym_stack.top();
	syntax_state = SNTX_LF_HT;
	syntax_level++;

	lex.get_token();
	while (is_type_specifier(token))
	{
		external_declatation(SC_LOCAL);
	}

	while ( token != TK_END)
	{
		statement(bsym, csym);
	}

	syntax_state = SNTX_LF_HT;
	
	sym_pop(local_sym_stack, s);
	lex.get_token();
}


void Syntax::expression_statement()
{
	if (token != TK_SEMICOLON)
	{
		expression();
		codegen.operand_pop();
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::if_statement(int* bsym, int *csym)
{
	int a = 0, b = 0;
	
	syntax_state = SNTX_SP;
	lex.get_token();
	lex.skip(TK_OPENPA);
	expression();
	syntax_state = SNTX_LF_HT;
	lex.skip(TK_CLOSEPA);


	a = codegen.gen_jcc(0);//reserve a jump, will fill back
	statement(bsym, csym);
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		lex.get_token();
		b = codegen.gen_jmpforward(0);
		backpatch(a, codegen.ind);
		statement(bsym, csym);
		backpatch(b, codegen.ind);
	}
	else
		backpatch(a, codegen.ind);
}

void Syntax::for_statement(int* bsym, int *csym)
{
	int a, b, c, d, e;
	lex.get_token();
	lex.skip(TK_OPENPA);
	if (token != TK_SEMICOLON)
	{
		expression();
		codegen.operand_pop();
	}

	lex.skip(TK_SEMICOLON);
	
	d = codegen.ind;
	c = codegen.ind;
	a = 0;
	b = 0;

	if (token != TK_SEMICOLON)
	{
		expression();
		a = codegen.gen_jcc(0);
	}

	lex.skip(TK_SEMICOLON);
	if (token != TK_CLOSEPA)
	{
		e = codegen.gen_jmpforward(0);
		c = codegen.ind;
		expression();
		codegen.operand_pop();
		codegen.gen_jmpbackward(c);
		backpatch(e, codegen.ind);
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_CLOSEPA);
	statement(&a, &b);

	codegen.gen_jmpbackward(c);
	backpatch(a, codegen.ind);
	backpatch(b, c);
}

void Syntax::continue_statement(int *csym)
{
	if (!csym)
		error("cannot use continue in this");
	
	*csym = codegen.gen_jmpforward(*csym);

	lex.get_token();
	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::break_statement(int *bsym)
{
	if (!bsym)
		error("cannot use break in this");
	*bsym = codegen.gen_jmpforward(*bsym);
	lex.get_token();
	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::return_statement()
{
	syntax_state = SNTX_DELAY;
	lex.get_token();
	if ( token == TK_SEMICOLON)
	{
		syntax_state = SNTX_NUL;
	}
	else
	{
		syntax_state = SNTX_SP;
	}

	lex.syntax_indent();

	if (token != TK_SEMICOLON)
	{
		expression();
		Operand* top = codegen.operand_stack.top();
		codegen.load_1(REG_IRET, top);
		codegen.operand_pop();
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
	codegen.rsym = codegen.gen_jmpforward(codegen.rsym);
}

void Syntax::expression()
{
	while (true)
	{
		assignment_expression();
		if (token != TK_COMMA)
			break;
		codegen.operand_pop();
		lex.get_token();
	}
}

void Syntax::assignment_expression()
{
	equality_expression();
	if (token == TK_ASSIGN)
	{
		codegen.check_lvalue();
		lex.get_token();
		assignment_expression();
		codegen.store0_1();
	}
}

void Syntax::equality_expression()
{
	int t = 0;
	relational_expression();
	while (token == TK_EQ || token == TK_NEQ)
	{
		t = token;
		lex.get_token();
		relational_expression();
		codegen.gen_op(t);
	}
}

void Syntax::relational_expression()
{
	int t = 0;
	additive_expression();
	while (token == TK_LT || token == TK_LEQ || token == TK_GT || token == TK_GEQ)
	{
		t = token;
		lex.get_token();
		additive_expression();
		codegen.gen_op(t);
	}
}

void Syntax::additive_expression()
{
	int t = 0;
	mutiplicative_expression();
	while (token == TK_PLUS || token == TK_MINUS)
	{
		t = token;
		lex.get_token();
		mutiplicative_expression();
		codegen.gen_op(t);
	}
}

void Syntax::mutiplicative_expression()
{
	int t = 0;
	unary_expression();
	while ( token == TK_STAR || token == TK_DIVIDE || token == TK_MOD)
	{
		t = token;
		lex.get_token();
		unary_expression();
		codegen.gen_op(t);
	}
}

void Syntax::unary_expression()
{
	switch (token)
	{
	case TK_AND:
		{
			lex.get_token();
			unary_expression();
			Operand* top = codegen.operand_stack.top();
			if ( (top->type.t & T_BTYPE) != T_FUNC && 
				 !(top->type.t & T_ARRAY)
				)
			{
				codegen.cancel_lvalue();
			}

			mk_pointer(&top->type);
		}
		break;
	case TK_STAR:
		lex.get_token();
		unary_expression();
		codegen.indirection();
		break;
	case TK_PLUS:
		lex.get_token();
		unary_expression();
		break;
	case TK_MINUS:
		lex.get_token();
		codegen.operand_push(&int_type, SC_GLOBAL, 0);
		unary_expression();
		codegen.gen_op(TK_MINUS);
		break;
	case KW_SIZEOF:
		sizeof_expression();
		break;
	default:
		postfix_expression();
		break;
	}
}

void Syntax::sizeof_expression()
{
	int align, size;
	Type type;

	lex.get_token();
	lex.skip(TK_OPENPA);
	type_specifier(&type);
	lex.skip(TK_CLOSEPA);

	size = type_size(&type, &align);
	if (size < 0)
		error("calculate type size failed!");

	codegen.operand_push(&int_type, SC_GLOBAL, size);
}

void Syntax::postfix_expression()
{
	Symbol *s = NULL;
	primary_expression();
	while (true)
	{
		if (token == TK_DOT || token == TK_POINTSTO)
		{
			if (token == TK_POINTSTO)
				codegen.indirection();
			codegen.cancel_lvalue();
			lex.get_token();
			Operand *top = codegen.operand_stack.top();
			if ((top->type.t & T_BTYPE) != T_STRUCT)
				expect("struct variable");
			s = top->type.ref;
			token |= SC_MEMBER;

			while ( ( s = s->next) != NULL)
			{
				if (s->v == token)
					break;
			}

			if (!s)
				error("no this member variable:%s", lex.get_tkstr(token & ~SC_MEMBER));
			// member list address = struct variable pointer + member variable offset
			top->type = char_pointer_type;
			codegen.operand_push(&int_type, SC_GLOBAL, s->c);
			codegen.gen_op(TK_PLUS);

			// change type to member variable data type
			top->type = s->type;

			top = codegen.operand_stack.top();
			// array variable cannot be lvalue
			if ( !(top->type.t & T_ARRAY))
			{
				top->r |= SC_LVAL;
			}

			lex.get_token();
		}
		else if (token == TK_OPENBR)
		{
			lex.get_token();
			expression();
			codegen.gen_op(TK_PLUS);
			codegen.indirection();
			lex.skip(TK_CLOSEBR);
		}
		else if (token == TK_OPENPA)
		{
			argument_expression_list();
		}
		else
			break;
	}
}

void Syntax::primary_expression()
{
	int t, r, addr;
	Type type;
	Symbol *s;
	Section *sec = NULL;

	switch (token)
	{
	case TK_CINT:
	case TK_CCHAR:
		lex.get_token();
		codegen.operand_push(&int_type, SC_GLOBAL, tkvalue);
		break;
	case TK_CSTR:
		//lex.get_token();
		t = T_CHAR;
		type.t = t;
		mk_pointer(&type);
		type.t |= T_ARRAY;
		sec = allocate_storage(&type, SC_GLOBAL, 2, 0, &addr);
		var_sym_put(&type, SC_GLOBAL, 0, addr);
		initializer(&type, addr, sec);
		break;
	case TK_OPENPA:
		lex.get_token();
		expression();
		lex.skip(TK_CLOSEPA);
		break;
	default:
		t = token;
		lex.get_token();
		if (t < TK_IDENT)
			expect("identify or constant");
		
		s = sym_search(t);
		if (!s)
		{
			if (token != TK_OPENPA)
				error("'%s' undeclare", lex.get_tkstr(t));

			s = func_sym_push(t, &default_func_type);//允许函数不声明，直接引用
			s->r = SC_GLOBAL | SC_SYM; 
		}
		r = s->r;
		codegen.operand_push(&s->type, r, s->c);
		Operand *top = codegen.operand_stack.top();
		// symbol reference, operand must record symbol address
		if (top->r & SC_SYM)
		{
			top->sym = s;
			top->value = 0;
		}

		break;
	}
}

void Syntax::argument_expression_list()
{
	Operand ret;
	Symbol *s, *sa;
	int nb_args;
	Operand* top = codegen.operand_stack.top();
	s = top->type.ref;
	lex.get_token();
	sa = s->next;		// first parameter
	nb_args = 0;
	ret.type = s->type;
	ret.r = REG_IRET;
	ret.value = 0;

	if (token != TK_CLOSEPA)
	{
		for (;;)
		{
			assignment_expression();
			nb_args++;
			if (sa)
				sa = sa->next;
			if (token == TK_CLOSEPA)
				break;
			lex.skip(TK_COMMA);
		}
	}

	if (sa)
		error("real argument less than function formal parameter");

	lex.skip(TK_CLOSEPA);
	codegen.gen_invoke(nb_args);
	codegen.operand_push(&ret.type, ret.r, ret.value);
}

Section* Syntax::allocate_storage(Type* type, int r, int has_init, int v, int *addr)
{
	int size = 0, align = 0;
	Section* sec = NULL;
	size = type_size(type, &align);

	if (size < 0)
	{
		if (type->t & T_ARRAY && type->ref->type.t == T_CHAR)
		{
			type->ref->c = lex.tkstr.length() + 1;
			size = type_size(type, &align);
		}
		else
			error("type size unknown");
	}

	// if the variable is local variable,allocate memory from stack
	if ( (r & SC_VALMASK) == SC_LOCAL)
	{
		codegen.loc = calc_align(codegen.loc - size, align);
		*addr = codegen.loc;
	}
	else
	{
		if (has_init == 1) // initialize global variable to .data section
			sec = coff.sec_data;
		else if (has_init == 2) // initialize read only variable to .rdata section
			sec = coff.sec_rdata;
		else
			sec = coff.sec_bss; // the uninitialized variable

		sec->data_offset = calc_align(sec->data_offset, align);
		*addr = sec->data_offset;
		sec->data_offset += size;

		//add memory space to section
		if (sec->sh.Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA
			&& sec->data_offset > sec->data_allocated)
			coff.section_realloc(sec, sec->data_offset);

		if (v == 0) // constant string
		{
			codegen.operand_push(type, SC_GLOBAL | SC_SYM, *addr);
			Operand* top = codegen.operand_stack.top();
			top->sym = sym_sec_rdata;
		}
	}
	return sec;
}

void Syntax::backpatch(int t, int a)
{
	int n, *ptr;
	while (t)
	{
		ptr = reinterpret_cast<int*>(coff.sec_text->data + t);
		n = *ptr;
		*ptr = a - t - 4;
		t = n;
	}
}

void Syntax::mk_pointer(Type* t)
{
	Symbol* s = NULL;
	s = sym_push(SC_ANOM, t, 0, -1);
	t->t = T_PTR;
	t->ref = s;
}